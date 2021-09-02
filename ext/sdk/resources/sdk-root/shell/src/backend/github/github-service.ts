import { injectable, inject } from 'inversify';
import { Octokit } from "@octokit/rest";
import { ApiContribution } from 'backend/api/api-contribution';
import { handlesClientCallbackEvent } from 'backend/api/api-decorators';
import { LogService } from 'backend/logger/log-service';
import { githubApi } from 'shared/api.events';
import { APIRQ } from 'shared/api.requests';

export const githubRepositoryUrlRegexp = /github.com\/([^\/]+)\/([^\/?]+)/;

@injectable()
export class GithubService implements ApiContribution {
  getId() {
    return 'ReleaseApi';
  }

  @inject(LogService)
  protected readonly logService: LogService;

  private octokit: Octokit = new Octokit({
    userAgent: 'citizenfx/fivem FxDK',
  });

  async listReleases(owner: string, repo: string) {
    return this.octokit.repos.listReleases({
      owner,
      repo,
    });
  }

  @handlesClientCallbackEvent(githubApi.fetchReleases)
  async fetchReleases(request: APIRQ.FetchReleases): Promise<APIRQ.FetchReleasesResponse> {
    this.logService.log('Fetching github releases', request);

    let success = false;
    let releases: APIRQ.ReleaseInfo[] = [];

    const match = request.repoUrl.match(githubRepositoryUrlRegexp);

    if (match) {
      try {
        const releasesResult = await this.listReleases(match[1], match[2].replace(/\.git$/, ''));

        if (releasesResult.status >= 200 && releasesResult.status < 299) {
          releases = releasesResult.data.map(release => {
            const info: APIRQ.ReleaseInfo = {
              name: release.name || '',
              body: release.body || '',
              createdAt: release.created_at,
              downloadUrl: release.assets?.[0]?.browser_download_url ?? release.zipball_url,
            };

            return info;
          });

          success = true;
        }
      } catch (e) {
        this.logService.log('Error while fetching github releases', e.toString());
      }
    }

    return {
      success,
      releases,
    };
  }
}
