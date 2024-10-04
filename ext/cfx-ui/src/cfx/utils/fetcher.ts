type TFetch = typeof fetch;

const originalFetch = fetch;
export namespace fetcher {
  export class HttpError extends Error {
    static is(error: Error): error is HttpError {
      return error instanceof HttpError;
    }

    public status: number;

    public statusText: string;

    constructor(public response: Response) {
      super(`Request to ${response.url} failed with status code ${response.status}`);

      this.status = response.status;
      this.statusText = response.statusText;
    }

    async readJsonBody<T>(): Promise<T | null> {
      if (this.response.bodyUsed) {
        return null;
      }

      try {
        return await this.response.json();
      } catch (e) {
        return null;
      }
    }
  }

  export class JsonParseError extends Error {
    static is(error: Error): error is JsonParseError {
      return error instanceof JsonParseError;
    }

    constructor(
      public originalString: string,
      error: Error,
    ) {
      super(`Invalid json "${originalString}", ${error.message}`);

      // Preserve stack
      this.stack = error.stack;
    }
  }

  // Like normal fetch, but will throw an error if !response.ok as well
  // so we can uniformly handle them just like errored ones, woo
  export async function fetch(...args: Parameters<TFetch>): ReturnType<TFetch> {
    const response = await originalFetch(...args);

    if (!response.ok) {
      throw new HttpError(response);
    }

    return response;
  }

  export async function json<T = any>(...args: Parameters<TFetch>): Promise<T> {
    const response = await fetch(...args);

    try {
      return await response.json();
    } catch (e) {
      throw new JsonParseError(response.bodyUsed
        ? 'BODY UNAVAILABLE'
        : await response.text(), e);
    }
  }

  export async function text(...args: Parameters<TFetch>): Promise<string> {
    return (await fetch(...args)).text();
  }

  export async function arrayBuffer(...args: Parameters<TFetch>): Promise<ArrayBuffer> {
    return (await fetch(...args)).arrayBuffer();
  }

  export async function typedArray<T extends new(
    buffer: ArrayBuffer) => any>(
    Ctor: T,
    ...args: Parameters<TFetch>
  ): Promise<InstanceType<T>> {
    const ab = await arrayBuffer(...args);

    return new Ctor(ab);
  }
}
