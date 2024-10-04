/* eslint-disable space-before-function-paren */
export function stopPropagation<T extends { stopPropagation: () => void }>(cb: (e: T) => void): (e: T) => void {
  return (e: T) => {
    e.stopPropagation();

    cb(e);
  };
}

export function preventDefault<T extends { preventDefault: () => void }>(cb: (e: T) => void): (e: T) => void {
  return (e: T) => {
    e.preventDefault();

    cb(e);
  };
}
