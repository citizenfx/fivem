export const fastRandomId = (): string => {
  return (Math.random() * 0xffffffff | 0).toString(16);
}
