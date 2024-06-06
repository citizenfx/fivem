export function createEnumChecker<T extends string, TEnumValue extends string>(enumVariable: {
  [key in T]: TEnumValue;
}) {
  const enumValues = Object.values(enumVariable);

  return (value: string): value is TEnumValue => enumValues.includes(value);
}
