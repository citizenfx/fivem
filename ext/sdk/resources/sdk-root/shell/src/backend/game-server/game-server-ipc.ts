import { AppendableBuffer } from "backend/appendable-buffer";
import { Deferred } from "backend/deferred";
import { DisposableObject } from "backend/disposable-container";
import net from 'net';
import { TextDecoder, TextEncoder } from 'util';

enum PacketType {
  RPC_CALL,
  RPC_RESPONSE,
  RPC_ERROR_RESPONSE,
  EVENT,
}

function randomUInt32(): number {
  return (Math.random() * 0x7FffFFff) | 0;
}

export class GameServerIPC implements DisposableObject {
  private eventListeners: Record<string, Function> = Object.create(null);
  private rpcDeferreds: Record<number, Deferred<any>> = Object.create(null);
  private closeHandler: (() => void) = () => {};

  public readonly connection = new Deferred<void>();

  rpc<PayloadType, ResponseType>(methodName: string, payload: PayloadType): Promise<ResponseType> {
    const callId = randomUInt32();
    const deferred = this.rpcDeferreds[callId] = new Deferred();

    this.emitRpc(methodName, callId, payload);

    return deferred.promise;
  }

  event<PayloadType>(eventName: string, payload?: PayloadType) {
    this.emitEvent(eventName, payload);
  }

  onEvent<PayloadType>(eventName: string, cb: (payload: PayloadType) => void): () => void {
    this.eventListeners[eventName] = cb;

    return () => delete this.eventListeners[eventName];
  }

  onClose(cb: () => void) {
    this.closeHandler = cb;
  }

  async start(pipeAppendix: string = randomUInt32().toString(16)): Promise<string> {
    const listenDeferred = new Deferred<string>();

    this.netServer.on('connection', (socket) => {
      this.netConn = socket;

      this.netConn.on('close', this.closeHandler);
      this.netConn.on('data', (data) => this.handleData(data));

      this.writeQueue.forEach((packet) => {
        this.netConn?.write(packet);
      });

      this.connection.resolve();
    });

    this.netServer.listen(
      `\\\\.\\pipe\\cfx-fxdk-fxserver-ipc-${pipeAppendix}`,
      () => listenDeferred.resolve(pipeAppendix),
    );

    return listenDeferred.promise;
  }

  dispose() {
    this.netServer.close();

    this.rpcDeferreds = Object.create(null);
    this.eventListeners = Object.create(null);

    this.closeHandler = () => {};
    this.buffer = null;
  }

  private netServer: net.Server = net.createServer();
  private netConn: net.Socket | null = null;
  private writeQueue: Buffer[] = [];
  private buffer: AppendableBuffer | null = new AppendableBuffer();
  private textEncoder = new TextEncoder();
  private textDecoder = new TextDecoder('utf8');
  private pendingPacketSize = 0;
  private handleData(buffer: Buffer) {
    if (!this.buffer) {
      return;
    }

    this.buffer.append(buffer);

    // While we have data to read - yeah
    while (this.buffer.length >= this.pendingPacketSize) {
      if (this.buffer.length === 0) {
        return;
      }

      // We have some data - init packet reading by fetching packet size
      if (this.pendingPacketSize === 0) {
        // Not enough data to even read packet size
        if (this.buffer.length < 4) {
          return;
        }

        this.pendingPacketSize = this.buffer.readUInt32();
      }

      // Not enough data to read packet
      if (this.buffer.length < this.pendingPacketSize) {
        return;
      }

      const packet = this.textDecoder.decode(this.buffer.readBuffer(this.pendingPacketSize));

      this.pendingPacketSize = 0;

      this.handlePacket(packet);
    }
  }

  private emitEvent<PayloadType>(eventName: string, payload: PayloadType) {
    this.emit([PacketType.EVENT, eventName, payload]);
  }

  private emitRpc<PayloadType>(methodName: string, callId: number, payload: PayloadType) {
    this.emit([PacketType.RPC_CALL, methodName, callId, payload]);
  }

  private emit<DataType>(data: DataType) {
    const dataStr = JSON.stringify(data);
    const dataPacked = this.textEncoder.encode(dataStr);

    const packet = Buffer.alloc(dataPacked.length + 4);

    packet.writeUInt32LE(dataPacked.length, 0);
    packet.set(dataPacked, 4);

    if (!this.netConn) {
      this.writeQueue.push(packet);
    } else {
      this.netConn.write(packet);
    }
  }

  private handlePacket(packetRaw: string) {
    const packet = JSON.parse(packetRaw);

    if (!Array.isArray(packet)) {
      throw new Error('Invalid packet, expected array');
    }

    switch (packet[0]) {
      case PacketType.EVENT: {
        const [, eventName, payload] = packet;

        this.eventListeners[eventName]?.(payload);

        break;
      }

      case PacketType.RPC_ERROR_RESPONSE: {
        const [, callId, error] = packet;

        const deferred = this.rpcDeferreds[callId];
        if (deferred) {
          delete this.rpcDeferreds[callId];

          deferred.reject(new Error(error));
        }

        break;
      }

      case PacketType.RPC_RESPONSE: {
        const [, callId, response] = packet;

        const deferred = this.rpcDeferreds[callId];
        if (deferred) {
          delete this.rpcDeferreds[callId];

          deferred.resolve(response);
        }

        break;
      }

      default: {
        console.log('Unsupported IPC packet type', packet[0]);
      }
    }
  }
}
