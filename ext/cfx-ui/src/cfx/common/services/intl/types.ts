// @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/PluralRules/select#return_value
export enum PluralRule {
  Zero = 'zero',
  One = 'one',
  Two = 'two',
  Few = 'few',
  Many = 'many',
  Other = 'other',
}

export type PluralKeys = Partial<Record<PluralRule, string>>;
