import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { inject, injectable } from "inversify";
import { formatDateForFilename } from "utils/date";
import Flatbush from 'flatbush';
import { WEEntityMatrixIndex, WEMap } from "./world-editor-types";
import { concurrently } from "utils/concurrently";
import { joaat } from "utils/joaat";
import { eulerFromMatrix } from "shared/math";
import { WorldEditorArchetypesService } from "./world-editor-archetypes-service";
import { FXWORLD_FILE_EXT } from "fxdk/contrib/assets/fxworld/common/fxworld-types";

const WORLD_EDITOR_MAP_INDEX_RUNTIME = require('./world-editor-map-runtime/map-index.raw.js');
const WORLD_EDITOR_MAP_ADDITIONS_RUNTIME = require('./world-editor-map-runtime/map-additions.raw.js');

export interface WorldEditorMapCompileRequest {
  mapFilePath: string,
  targetPath: string,
}

interface CompilationData {
  map: WEMap,
  name: string,
  compiled: string[],
}

@injectable()
export class WorldEditorMapCompiler {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(WorldEditorArchetypesService)
  protected readonly archetypesService: WorldEditorArchetypesService;

  async compile(request: WorldEditorMapCompileRequest) {
    await this.prepareCompilationSite(request);

    const compilationData: CompilationData = {
      map: await this.fsService.readFileJson(request.mapFilePath),
      name: this.getMapName(request.mapFilePath),
      compiled: [],
    };

    await this.compileHeader(compilationData);
    await this.compilePatches(compilationData);
    await this.compileAdditions(compilationData);

    await this.writeCompiled(request, compilationData);
  }

  private async prepareCompilationSite(request: WorldEditorMapCompileRequest) {
    if (!(await this.fsService.statSafe(request.targetPath))) {
      await this.fsService.mkdirp(request.targetPath);
    }
  }

  private async compileHeader(data: CompilationData) {
    data.compiled.push(
      '// AUTOMATICALLY GENERATED FILE',
      '// ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN',
      '//',
      `// Compiled at ${formatDateForFilename(new Date())}`,
      'setTimeout(EnableEditorRuntime, 0);',
      `on('onResourceStop', (name) => { if (name === GetCurrentResourceName()) DisableEditorRuntime(); });`,
    );
  }

  private async compilePatches(data: CompilationData) {
    data.compiled.push('// Map patches');

    data.compiled.push([
      'function applyPatch(md,_e,u){',
      'const e=GetEntityIndexFromMapdata(md,_e);',
      `if(e===-1)return console.error('Failed to get entity index from mapdata',{mapdataHash:_md,mapdataIndex:md,entityHash:_e});`,
      `UpdateMapdataEntity(md,e,u)`,
      '}',
    ].join(''));

    if (Object.keys(data.map.patches).length) {
      // Compiling patches cache
      const vars: string[] = [];
      const loaders: string[] = [];

      Object.entries(data.map.patches).forEach(([mapdata, entities]) => {
        loaders.push(
          `case ${mapdata}:{`,
          'const m=GetMapdataFromHashKey(md);',
        );

        Object.entries(entities).forEach(([entity, patch]) => {
          const values = {
            position: JSON.stringify([
              patch.mat[12],
              patch.mat[13],
              patch.mat[14],
            ]),
            matrix: JSON.stringify(patch.mat),
          };

          const varName = getPatchVarName(mapdata, entity);
          const valuesString = `{position:${values.position},matrix:${values.matrix}}`;

          vars.push(
            `${varName}=${valuesString}`,
          );
          loaders.push(
            `applyPatch(m,${entity},${varName})`,
          );
        });

        loaders.push(
          'break}',
        );
      });

      data.compiled.push(`const`);
      data.compiled.push(vars.join(',\n') + ';');

      data.compiled.push(
        `on('mapDataLoaded',(md)=>{switch(md){`,
        loaders.join('\n'),
        '}})',
      );
    }
  }

  private async compileAdditions(data: CompilationData) {
    const additionsLength = Object.keys(data.map.additions).length;
    if (!additionsLength) {
      return;
    }

    const index = new Flatbush(Object.keys(data.map.additions).length, 16, Float32Array);

    const additions: string[] = [];

    for (const addition of Object.values(data.map.additions)) {
      let viewDistance = addition.vd || 0;

      // last attempt to get correct view distance
      // mainly for old maps that was created before we introduced saving of view distance
      if (typeof addition.vd === 'undefined' && typeof addition.mdl === 'string') {
        viewDistance = this.archetypesService.getArchetypeLodDist(addition.mdl) || 0;
      }

      // we add addition to index as a square, so we make it a tad bigger to compensate for it not being a circle
      // so it'd get created earlier than LOD kicks in
      const size = viewDistance / 2 + 5;

      const x = addition.mat[WEEntityMatrixIndex.AX];
      const y = addition.mat[WEEntityMatrixIndex.AY];

      additions[index.add(x - size, y - size, x + size, y + size)] = JSON.stringify([
        // number type of addition model is deprecated and this only to support old maps with it being a number
        typeof addition.mdl === 'string'
          ? joaat(addition.mdl)
          : addition.mdl,
        addition.mat,
        eulerFromMatrix(addition.mat),
        addition.evcreated || '',
        addition.evdeleted || '',
      ]);
    }

    index.finish();

    data.compiled.push(
      WORLD_EDITOR_MAP_INDEX_RUNTIME,
      '// Map additions',
      `const mai=Flatbush.from(new Uint8Array(${JSON.stringify(Array.from(new Uint8Array(index.data)))}).buffer)`,
      `const mad=[`,
      additions.join(',\n'),
      `]`,
      `const additionCreatedEventName='${data.name}:additionCreated', additionDeletedEventName='${data.name}:additionDeleted'`,
      WORLD_EDITOR_MAP_ADDITIONS_RUNTIME,
    );
  }

  private async writeCompiled(request: WorldEditorMapCompileRequest, data: CompilationData) {
    const filePath = this.fsService.joinPath(request.targetPath, 'map.js');
    const manifestPath = this.fsService.joinPath(request.targetPath, 'fxmanifest.lua');

    await concurrently(
      this.fsService.writeFile(filePath, data.compiled.join('\n')),
      this.fsService.writeFile(manifestPath, [
        '-- AUTOMATICALLY GENERATED FILE',
        '-- ANY CHANGES TO THIS FILE WILL BE OVERWRITTEN',
        '',
        `fx_version 'bodacious'`,
        `game 'gta5'`,
        `client_script 'map.js'`,
      ].join('\n')),
    );
  }

  private getMapName(mapFilePath: string): string {
    return this.fsService.basename(mapFilePath, FXWORLD_FILE_EXT);
  }
}

function getPatchVarName(mapdata: string | number, entity: string | number): string {
  return `mp${hashToVariableName(mapdata)}_${hashToVariableName(entity)}`;
}

function hashToVariableName(n: string | number): string {
  const ns = n.toString();

  if (ns[0] === '-') {
    return `_${ns.slice(1)}`;
  }

  return ns;
}
