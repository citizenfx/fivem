import { master } from './master';

export function decodeServer(frame: Uint8Array): master.IServer {
  return master.Server.decode(frame);
}
