import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { ShellBackend } from "backend/shell-backend";
import { inject, injectable, postConstruct } from "inversify";
import { ARCHETYPE_LOD_DIST } from "./world-editor-archetypes-constants";
import { Archetypes } from "./world-editor-archetypes-types";

@injectable()
export class WorldEditorArchetypesService {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  private archetypes: Archetypes | void;
  private archetypesString: string | void;

  @postConstruct()
  protected initialize() {
    this.shellBackend.expressApp.get('/archetypes.json', async (_req, res) => {
      const valid = await this.isValid();
      if (!valid) {
        return res.sendStatus(404);
      }

      res.sendFile(this.configService.archetypesCollectionPath);
    });
  }

  public async refresh() {
    if (!(await this.isValid())) {
      return;
    }

    this.archetypesString = await this.fsService.readFileString(this.configService.archetypesCollectionPath);
    this.archetypes = JSON.parse(this.archetypesString);
  }

  public getArchetypeLodDist(archetypeName: string): number | undefined {
    if (!this.archetypes) {
      return;
    }

    return this.archetypes.archetypes[archetypeName]?.[ARCHETYPE_LOD_DIST];
  }

  public async isValid(): Promise<boolean> {
    if (!(await this.fsService.statSafe(this.configService.archetypesCollectionPath))) {
      return false;
    }

    const expectedHeader = '{"____v":1';
    const fileHeader = await this.fsService.readFileFirstBytes(
      this.configService.archetypesCollectionPath,
      expectedHeader.length,
    );

    if (fileHeader !== expectedHeader) {
      return false;
    }

    return true;
  }
}
