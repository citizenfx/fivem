export function invariant(condition: unknown, errorMessage: string): asserts condition {
  if (!condition) {
    throw new Error(errorMessage);
  }
}
