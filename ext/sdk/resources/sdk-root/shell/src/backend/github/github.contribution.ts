import { registerApiContribution } from 'backend/api/api.extensions';
import { registerSingleton } from 'backend/container-access';
import { GithubService } from './github-service';

registerApiContribution(
  registerSingleton(GithubService)
);
