import * as cp from 'child_process';
import { OutputListener, OutputChannelProvider } from 'backend/output/output-types';
import { Deferred } from 'backend/deferred';
import { getContainer } from 'backend/container-access';
import { ConfigService } from 'backend/config-service';

export type ShellCommandDataListener = (data: Buffer) => void;
export type ShellCommandErrorListener = (error: Error) => void;
export type ShellCommandCloseListener = (code: number, signal: NodeJS.Signals | null) => void;

export class ShellCommand implements OutputChannelProvider {
  private proc: cp.ChildProcess | null = null;

  private pidCheckInterval: NodeJS.Timeout;

  private stdoutListeners: Set<ShellCommandDataListener> = new Set();
  private stderrListeners: Set<ShellCommandDataListener> = new Set();

  private errorListeners: Set<ShellCommandErrorListener> = new Set();

  private closeListeners: Set<ShellCommandCloseListener> = new Set();

  get running(): boolean {
    return !!this.proc;
  }

  constructor(
    private readonly command: string,
    private readonly args: string[],
    private readonly cwd?: string,
  ) {
    if (!this.cwd) {
      this.cwd = process.cwd();
    }
  }

  getOutputChannelId(): string {
    return JSON.stringify([this.command, this.args, this.cwd]);
  }

  getOutputChannelLabel(): string {
    return `${this.cwd}: ${this.command} ${this.args.join(' ')}`.trim();
  }

  onOutputData(listener: OutputListener) {
    return this.onStdout(listener);
  }

  onStdout(listener: ShellCommandDataListener): () => void {
    this.stdoutListeners.add(listener);

    return () => this.stdoutListeners.delete(listener);
  }

  onStderr(listener: ShellCommandDataListener): () => void {
    this.stderrListeners.add(listener);

    return () => this.stderrListeners.delete(listener);
  }

  onError(listener: ShellCommandErrorListener): () => void {
    this.errorListeners.add(listener);

    return () => this.errorListeners.delete(listener);
  }

  onClose(listener: ShellCommandCloseListener): () => void {
    this.closeListeners.add(listener);

    return () => this.closeListeners.delete(listener);
  }

  writeStdin(data: Buffer | string) {
    if (this.proc) {
      this.proc.stdin?.write(data);
    }
  }

  async start() {
    this.proc = cp.spawn(this.command, this.args, {
      cwd: this.cwd,
      shell: true,
      windowsHide: true,
    });

    this.proc.stdout?.on('data', (data) => {
      this.stdoutListeners.forEach((listener) => listener(data));
    });

    this.proc.stderr?.on('data', (data) => {
      this.stdoutListeners.forEach((listener) => listener(data));
      this.stderrListeners.forEach((listener) => listener(data));
    });

    this.proc.on('error', (err) => {
      if (this.proc?.pid) {
        this.proc.kill('SIGTERM');
      } else {
        this.closeListeners.forEach((listener) => listener(Number.MIN_SAFE_INTEGER, null));
      }

      this.proc = null;

      if (this.pidCheckInterval) {
        clearInterval(this.pidCheckInterval);
      }

      this.errorListeners.forEach((listener) => listener(err));
    });

    this.proc.on('close', (code, signal) => {
      this.proc = null;

      if (this.pidCheckInterval) {
        clearInterval(this.pidCheckInterval);
      }

      this.closeListeners.forEach((listener) => listener(code, signal));
    });

    this.proc.unref();

    // who knows, right
    if (this.proc.pid) {
      return;
    }

    const startDeferred = new Deferred<void>();

    this.pidCheckInterval = setInterval(() => {
      if (this.proc) {
        if (this.proc.pid) {
          clearInterval(this.pidCheckInterval);
          startDeferred.resolve();
        }
      } else {
        clearInterval(this.pidCheckInterval);
        startDeferred.reject();
      }
    }, 100); // sounds like a reasonable interval

    return startDeferred.promise;
  }

  stop(): Promise<null | { code: number, signal: NodeJS.Signals }> {
    return new Promise((resolve) => {
      if (!this.proc) {
        this.closeListeners.forEach((listener) => listener(Number.MIN_SAFE_INTEGER, null));
        return resolve(null);
      }

      this.proc.on('close', (code, signal) => {
        resolve({ code, signal });
      });

      const { pid } = this.proc;

      cp.exec(`taskkill /pid ${pid} /T /F`, {
        cwd: getContainer().get(ConfigService).realCwd,
      }, (error) => {
        if (error) {
          console.error(`Failed to kill child process, pid: ${pid}`, error);
        }
      });
    });
  }
}
