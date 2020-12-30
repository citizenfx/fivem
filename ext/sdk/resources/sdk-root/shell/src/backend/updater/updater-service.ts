import { inject, injectable, named } from "inversify";
import { StatusService } from "backend/status/status-service";
import { AppContribution } from "backend/app/app-contribution";
import { updaterStatuses } from "shared/api.statuses";
import { ContributionProvider } from 'backend/contribution-provider';
import { UpdaterContribution } from './updater-contribution';

@injectable()
export class UpdaterService implements AppContribution {
  @inject(StatusService)
  protected readonly statusService: StatusService;

  @inject(ContributionProvider) @named(UpdaterContribution)
  protected readonly updaterContributions: ContributionProvider<UpdaterContribution>;

  async prepare() {
    await Promise.all(this.updaterContributions.getAll().map((contribution) => contribution.update()));

    // await this.simulateUpdate();
  }

  protected async simulateUpdate(): Promise<void> {
    await new Promise<void>((resolve) => {
      let steps = 0;
      const limit = 100;

      const sendStatus = () => {
        this.statusService.set(updaterStatuses.state, {
          completed: steps / limit,
          currentFileName: `Step ${steps}/${limit}`,
        });
      }

      const int = setInterval(() => {
        if (steps++ >= limit) {
          clearInterval(int);
          return resolve();
        }

        sendStatus();
      }, 50);
    });
  }
}
