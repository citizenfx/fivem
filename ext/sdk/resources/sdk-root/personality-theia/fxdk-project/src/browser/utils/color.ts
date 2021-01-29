import { joaat } from "./joaat";

export function hslForKey(key: string): [number, number, number] {
  return [joaat(key) % 360, 80, 40];
}
