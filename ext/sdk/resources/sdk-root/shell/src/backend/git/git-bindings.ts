import { interfaces } from 'inversify';
import { GitService } from './git-service';

export const bindGit = (container: interfaces.Container) => {
  container.bind(GitService).toSelf().inSingletonScope();
};
