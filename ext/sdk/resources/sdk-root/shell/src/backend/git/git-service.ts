import fs from 'fs';
import git from 'isomorphic-git';
import http from 'isomorphic-git/http/node';
import { LogService } from 'backend/logger/log-service';
import { inject, injectable } from 'inversify';
import { FsService } from 'backend/fs/fs-service';

const STATUS_MAPPING = {
  '000': 'the fuck?',
  '003': 'added',
  '020': '??',
  '022': 'added',
  '023': 'added',
  '100': 'deleted',
  '101': 'deleted',
  '103': 'modified',
  '110': 'deleted ??',
  '111': 'unchanged',
  '113': 'modified',
  '120': 'deleted ??',
  '121': 'modified',
  '122': 'modified',
  '123': 'modified',
};

@injectable()
export class GitService {
  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  clone(dir: string, repositoryURL: string) {
    return git.clone({
      fs,
      dir,
      http,
      url: repositoryURL,
    });
  }

  pullChanges(dir: string) {
    return git.pull({
      fs,
      dir,
      http,
      fastForwardOnly: true,
    });
  }

  fastForwrad(dir: string) {
    return git.fastForward({
      fs,
      dir,
      http,
    });
  }

  getRemotes(dir: string) {
    return git.listRemotes({ fs, dir });
  }

  listLocalBranches(dir: string): Promise<string[]> {
    return git.listBranches({ fs, dir });
  }

  async getRemoteUrl(dir: string, remoteName: string): Promise<string | void> {
    const remotes = await this.getRemotes(dir);
    const origin = remotes.find(({ remote }) => remote === remoteName);

    if (!origin) {
      return;
    }

    return origin.url;
  }

  async getOriginRemoteUrl(dir: string): Promise<string | void> {
    return this.getRemoteUrl(dir, 'origin');
  }

  async getRemoteBranchRef(dir: string, remoteName: string, branch: string): Promise<string | void>  {
    const url = await this.getRemoteUrl(dir, remoteName);
    if (!url) {
      return;
    }

    const remoteInfo = await git.getRemoteInfo({
      http,
      url,
    });

    const branchInfo = remoteInfo.refs?.heads?.[branch];
    if (!branchInfo || typeof branchInfo !== 'string') {
      return;
    }

    return branchInfo;
  }

  getLocalBranchRef(dir: string, branch: string): Promise<string | void> {
    return git.resolveRef({
      fs,
      ref: branch,
      dir,
    });
  }

  async getUncommitedChanges(dir: string): Promise<[string, string][]> {
    const matrix = await git.statusMatrix({ fs, dir });

    return matrix
      .filter(([_, head, workdir, stage]) => head !== 1 || workdir !== 1 || stage !== 1)
      .map(([file, head, workdir, stage]) => [file, STATUS_MAPPING[`${head}${workdir}${stage}`]]);
  }

  fetchRemote(dir: string, remote: string, branch: string = 'master') {
    return git.fetch({
      fs,
      http,
      dir,
      url: remote,
      ref: branch,
      singleBranch: true,
    });
  }

  async hasBranchDivertedFrom(dir: string, branchA: string, branchB: string): Promise<boolean> {
    const [branchACommitOid, branchBCommitOid] = await Promise.all([
      git.resolveRef({ fs, dir, ref: branchA }),
      git.resolveRef({ fs, dir, ref: branchB }),
    ]);

    const mergeBaseCommitOids = await git.findMergeBase({ fs, dir, oids: [branchACommitOid, branchBCommitOid] });
    if (mergeBaseCommitOids.length !== 1) {
      return true;
    }

    const [mergeBaseCommitOid] = mergeBaseCommitOids;

    const mergeBaseCommit = await git.readCommit({ fs, dir, oid: mergeBaseCommitOid });

    const [branchALog, branchBLog] = await Promise.all([
      git.log({ fs, dir, ref: branchA, since: new Date(mergeBaseCommit.commit.author.timestamp * 1000) }),
      git.log({ fs, dir, ref: branchB, since: new Date(mergeBaseCommit.commit.author.timestamp * 1000) }),
    ]);

    if (branchALog.length === 0) {
      return false;
    }

    return branchALog[0]?.oid !== branchBLog[0]?.oid;
  }

  hasBranchDirevertedFromRemote(dir: string, branch: string): Promise<boolean> {
    return this.hasBranchDivertedFrom(dir, branch, `remotes/origin/${branch}`);
  }
}
