export type TCFXID = string;

export function formatCFXID(userId: number | string): TCFXID {
  return `fivem:${userId}`;
}
