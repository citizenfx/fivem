export function markTime(label: string) {
  console.time(label);

  return () => console.timeEnd(label);
}
