export type TextColor = 'inherit' | 'main' | 'primary' | 'teal' | 'success' | 'warning' | 'error';

export type TextSize = 'xsmall' | 'small' | 'normal' | 'large' | 'xlarge' | 'xxlarge';

export type TextWeight = 'thin' | 'normal' | 'bold' | 'bolder';

export type TextOpacity = '0' | '25' | '50' | '75' | '100';

interface TextPropsBase {
  asDiv?: boolean;
  centered?: boolean;
  truncated?: boolean;

  /**
   * If it'll be a multiline text, use this to add extra spacing between lines
   */
  typographic?: boolean;

  /**
   * Allow users to select text
   */
  userSelectable?: boolean;

  uppercase?: boolean;

  size?: TextSize;
  weight?: TextWeight;

  family?: 'primary' | 'secondary' | 'monospace';

  children?: React.ReactNode;
  className?: string;
}

interface TextPropsFullControl extends TextPropsBase {
  color?: TextColor;
  opacity?: TextOpacity;
}

interface TextPropsColorToken extends TextPropsBase {
  colorToken: string;
}

export type TextProps = TextPropsFullControl | TextPropsColorToken;
