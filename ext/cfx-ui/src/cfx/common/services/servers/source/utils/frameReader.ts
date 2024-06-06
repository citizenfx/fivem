/* eslint-disable no-bitwise */
export class FrameReader {
  protected reader = this.stream.getReader();

  protected lastArray: Uint8Array | null = null;

  protected frameLength = -1;

  protected framePos = 0;

  constructor(
    protected stream: ReadableStream<Uint8Array>,
    protected onFrame: (frame: Uint8Array) => void,
    protected onEnd: () => void,
    // eslint-disable-next-line no-empty-function
  ) {}

  public read() {
    this.doRead();
  }

  private async doRead() {
    const {
      done,
      value,
    } = await this.reader.read();

    if (done || !value) {
      this.onEnd();

      return;
    }

    let array: Uint8Array = value;

    while (array.length > 0) {
      const start = 4;

      if (this.lastArray) {
        const newArray = new Uint8Array(array.length + this.lastArray.length);
        newArray.set(this.lastArray);
        newArray.set(array, this.lastArray.length);

        this.lastArray = null;

        array = newArray;
      }

      if (this.frameLength < 0) {
        if (array.length < 4) {
          this.lastArray = array;
          this.doRead();

          return;
        }

        this.frameLength = array[0] | (array[1] << 8) | (array[2] << 16) | (array[3] << 24);

        if (this.frameLength > 65535) {
          throw new Error('A too large frame was passed.');
        }
      }

      const end = 4 + this.frameLength - this.framePos;

      if (array.length < end) {
        this.lastArray = array;
        this.doRead();

        return;
      }

      const frame = softSlice(array, start, end);
      this.framePos += end - start;

      if (this.framePos === this.frameLength) {
        // reset
        this.frameLength = -1;
        this.framePos = 0;
      }

      this.onFrame(frame);

      // more in the array?
      if (array.length > end) {
        array = softSlice(array, end);
      } else {
        // continue reading
        this.doRead();

        return;
      }
    }
  }
}

function softSlice(arr: Uint8Array, start: number, end?: number): Uint8Array {
  return new Uint8Array(arr.buffer, arr.byteOffset + start, end && end - start);
}
