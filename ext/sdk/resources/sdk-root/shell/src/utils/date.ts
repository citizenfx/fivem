export const z = (n: number): string => n < 10 ? '0'+n : n.toString(10);

export function formatDateForFilename(date: Date): string {
  const year = date.getFullYear();
  const month = date.getMonth() + 1; // thanks js
  const day = date.getDate();

  const h = date.getHours();
  const m = date.getMinutes();
  const s = date.getSeconds();

  return `${year}-${z(month)}-${z(day)}T${z(h)}.${z(m)}.${z(s)}`;
}
