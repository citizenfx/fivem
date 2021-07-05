import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { inject, injectable } from "inversify";
import { formatDateForFilename } from "utils/date";
import Flatbush from 'flatbush';
import { WEMap } from "./world-editor-types";
import { concurrently } from "utils/concurrently";

const WORLD_EDITOR_MAP_INDEX_RUNTIME = require('./world-editor-map-runtime/map-index.raw.js');
const WORLD_EDITOR_MAP_ADDITIONS_RUNTIME = require('./world-editor-map-runtime/map-additions.raw.js');

export interface WorldEditorMapCompileRequest {
  mapFilePath: string,
  targetPath: string,
}

interface CompilationData {
  map: WEMap,
  compiled: string[],
}

@injectable()
export class WorldEditorMapCompiler {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  async compile(request: WorldEditorMapCompileRequest) {
    await this.prepareCompilationSite(request);

    const compilationData: CompilationData = {
      map: await this.fsService.readFileJson(request.mapFilePath),
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
      '// ANY CHANGES MADE TO THIS FILE WILL BE OVERWRITTEN',
      '//',
      `// Compiled at ${formatDateForFilename(new Date())}`,
      'setTimeout(() => EnableEditorRuntime(), 0)',
      WORLD_EDITOR_MAP_INDEX_RUNTIME,
      '',
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
          `case ${mapdata}:`,
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
          'break',
        );
      });

      data.compiled.push(`const ${vars[0]},`);
      data.compiled.push(vars.slice(1).join(',\n') + ';');

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
      const x = addition.mat[12];
      const y = addition.mat[13];

      additions[index.add(x, y, x, y)] = JSON.stringify([addition.hash, addition.mat]);
    }

    index.finish();

    data.compiled.push(
      '',
      '// Map additions',
      `const mai=Flatbush.from(new Uint8Array(${JSON.stringify(Array.from(new Uint8Array(index.data)))}).buffer)`,
      `const mad=[`,
      additions.join(',\n'),
      `]`,
      WORLD_EDITOR_MAP_ADDITIONS_RUNTIME,
    );
  }

  private async writeCompiled(request: WorldEditorMapCompileRequest, data: CompilationData) {
    const filePath = this.fsService.joinPath(request.targetPath, 'map.js');
    const manifestPath = this.fsService.joinPath(request.targetPath, 'fxmanifest.lua');

    await concurrently(
      this.fsService.writeFile(filePath, data.compiled.join('\n')),
      this.fsService.writeFile(manifestPath, `fx_version 'bodacious'; game 'gta5'; client_script 'map.js'`),
    );
  }
}

function getPatchVarName(mapdata: string | number, entity: string | number): string {
  return `mp${n2s(mapdata)}_${n2s(entity)}`;
}

function n2s(n: string | number): string {
  const ns = n.toString();

  if (ns[0] === '-') {
    return `_${ns.slice(1)}`;
  }

  return ns;
}
