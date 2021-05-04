export function endsWith(str: string, end: string): boolean {
  return str.lastIndexOf(end) === (str.length - end.length);
}
