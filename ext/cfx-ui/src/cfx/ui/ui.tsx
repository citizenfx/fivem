import { TextProps } from "./Text/Text";

export namespace ui {
  /**
   * Returns CSS value of quant value with applied multiplier
   */
  export function q(multiplier: number = 1): string {
    return `calc(var(--quant) * ${multiplier})`;
  }

  export function offset(size: 'none' | 'xsmall' | 'small' | 'normal' | 'large' | 'xlarge' | 'safezone' = 'normal'): string {
    return `var(--offset-${size})`;
  }

  export function fontSize(size: TextProps['size'] = 'normal'): string {
    return `var(--font-size-${size})`;
  }

  export function borderRadius(size: 'xsmall' | 'small' | 'normal' = 'normal'): string {
    return `var(--border-radius-${size})`;
  }

  export function color(name: string, variant?: string | number, alpha = 1.0): string {
    if (variant === 'pure') {
      variant = '';
    }
    if (typeof variant === 'number') {
      variant = `${variant}`;
    }

    return `rgba(var(--color-${name}${variant ? '-' + variant : ''}), ${alpha})`;
  }

  export namespace cls {
    export const fullWidth = 'util-full-width';
    export const fullHeight = 'util-full-height';

    export const zIndex9000 = 'util-z-index-9000';
  }

  export const px = (x: number | string) => `${x}px`;
  export const ch = (x: number | string) => `${x}ch`;
  export const em = (x: number | string) => `${x}em`;
  export const rem = (x: number | string) => `${x}rem`;
  export const vh = (x: number | string) => `${x}vh`;
  export const vw = (x: number | string) => `${x}vw`;
}
