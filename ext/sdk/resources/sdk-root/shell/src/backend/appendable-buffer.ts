class BufferNode {
  public start = 0;

  get length(): number {
    return this.buffer.length - this.start;
  }

  constructor(public buffer: Buffer) {

  }

  readUInt32(): number {
    if (this.length >= 4) {
      const num = this.buffer.readUInt32LE(this.start);

      this.start += 4;

      return num;
    }

    throw new Error(`BufferNode's remainig size is less than 4 bytes needed to read uin32_t`);
  }

  readBuffer(length: number): Buffer {
    if (this.length >= length) {
      const buffer = this.buffer.slice(this.start, this.start + length);

      this.start += length;

      return buffer;
    }

    throw new Error(`BufferNode's remaining size is less than ${length} bytes needed to read buffer`);
  }
}

export class AppendableBuffer {
  private nodes: BufferNode[] = [];

  get length(): number {
    return this.nodes.reduce((length, node) => length + node.length, 0);
  }

  append(buffer: Buffer) {
    this.nodes.push(new BufferNode(buffer));
  }

  readUInt32() {
    return this.readBuffer(4).readUInt32LE(0);
  }

  readBuffer(length: number): Buffer {
    let node = this.getNode();

    // Best case - head node has needed buffer size
    if (node.length >= length) {
      const buffer = node.readBuffer(length);

      this.shrinkNodes();

      return buffer;
    }

    const buffers: Buffer[] = [];

    let remainingLength = length;

    while (true) {
      const nodeLength = node.length;

      if (remainingLength > nodeLength) {
        remainingLength -= nodeLength;

        buffers.push(node.readBuffer(nodeLength));
      } else {
        buffers.push(node.readBuffer(remainingLength));
        break;
      }

      this.shrinkNodes();
      node = this.getNode();
    }

    return Buffer.concat(buffers, length);
  }

  private getNode(): BufferNode {
    const node = this.nodes[0];
    if (!node) {
      throw new Error(`No buffer nodes available`);
    }

    return node;
  }

  private shrinkNodes() {
    this.nodes = this.nodes.filter((node) => node.length > 0);
  }
}
