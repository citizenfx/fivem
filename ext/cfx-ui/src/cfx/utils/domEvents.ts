export function stopPropagation<T extends { stopPropagation: Function }>(cb: (e: T) => void): (e: T) => void {
  return (e: T) => {
    e.stopPropagation();

    cb(e);
  };
}

export function preventDefault<T extends { preventDefault: Function }>(cb: (e: T) => void): (e: T) => void {
  return (e: T) => {
    e.preventDefault();

    cb(e);
  };
}
