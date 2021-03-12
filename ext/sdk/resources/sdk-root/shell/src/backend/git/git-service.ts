import fs from 'fs';
import git from 'isomorphic-git';
import http from 'isomorphic-git/http/node';
import { LogService } from 'backend/logger/log-service';
import { inject, injectable } from 'inversify';
import { FsService } from 'backend/fs/fs-service';

@injectable()
export class GitService {
  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  clone(cwd: string, alias: string, repositoryURL: string) {
    return git.clone({
      fs,
      dir: this.fsService.joinPath(cwd, alias),
      http,
      url: repositoryURL,
    });
  }
}
