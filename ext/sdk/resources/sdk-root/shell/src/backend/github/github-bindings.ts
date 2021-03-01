import { bindApiContribution } from 'backend/api/api-contribution';
import { interfaces } from 'inversify';
import { GithubService } from './github-service';

export const bindGithub = (container: interfaces.Container) => {
  container.bind(GithubService).toSelf().inSingletonScope();
  bindApiContribution(container, GithubService);
};
