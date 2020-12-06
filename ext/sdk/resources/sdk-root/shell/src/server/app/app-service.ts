import { inject, injectable, named } from "inversify";
import { ConfigService } from "server/config-service";
import { LogService } from "server/logger/log-service";
import { AppStates } from "shared/api.types";
import { ContributionProvider } from "server/contribution-provider";
import { ShellBackend } from "server/shell-backend";
import { AppContribution } from "./app-contribution";
import { AppStateService } from "./app-state-service";
import { setupWellKnownPaths } from "./setup-wellknown-paths";

@injectable()
export class AppService {
  @inject(AppStateService)
  protected readonly appStateService: AppStateService;

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ContributionProvider) @named(AppContribution)
  protected readonly appContributions: ContributionProvider<AppContribution>;

  /**
   * Executes app boot sequence
   */
  async startContributions() {
    this.logService.log('Starting app...');

    setupWellKnownPaths(this.configService);

    const contributions = this.appContributions.getAll();

    this.logService.log('Booting...');

    this.appStateService.toState(AppStates.booting);
    await Promise.all(contributions.map((appContribution) => appContribution.boot?.()));

    this.logService.log('Opening window...');
    emit('sdk:openBrowser', this.configService.sdkUrl);

    this.logService.log('Preparing...');
    this.appStateService.toState(AppStates.preparing);
    await Promise.all(contributions.map((appContribution) => appContribution.prepare?.()));

    this.logService.log('Before app start...');
    await Promise.all(contributions.map((appContribution) => appContribution.beforeAppStart?.()));
    this.appStateService.toState(AppStates.ready);
    this.logService.log('After app start...');
    await Promise.all(contributions.map((appContribution) => appContribution.afterAppStart?.()));

    this.logService.log('App started!');
  }
}
