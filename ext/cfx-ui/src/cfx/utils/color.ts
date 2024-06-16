/* eslint-disable no-bitwise */
import Color from 'color';
import md5 from 'js-md5';

function shouldChangeColor(color: Color): boolean {
  const rgb = color.rgb().array();
  const val = 765 - (rgb[0] + rgb[1] + rgb[2]);

  return val < 250 || val > 700;
}

function djb2(str: string): number {
  let hash = 5381;

  for (let i = 0; i < str.length; i++) {
    hash = (hash << 5) + hash + str.charCodeAt(i);
  }

  return hash;
}

function hashStringToColor(str: string): string {
  const hash = djb2(str);
  const r = (hash & 0xff0000) >> 16;
  const g = (hash & 0x00ff00) >> 8;
  const b = hash & 0x0000ff;

  return `#${`0${r.toString(16)}`.substr(-2)}${`0${g.toString(16)}`.substr(-2)}${`0${b.toString(16)}`.substr(-2)}`;
}

function getMatchingColor(firstColor: Color): Color {
  let color = firstColor;

  if (color.isDark()) {
    color = color.saturate(0.3).rotate(90);
  } else {
    color = color.desaturate(0.3).rotate(90);
  }

  if (shouldChangeColor(color)) {
    color = color.rotate(-200).saturate(0.5);
  }

  return color;
}

export interface IGradient {
  from: string;
  to: string;
}

export function getGradientFor(key: string): IGradient {
  const hashStr: string = md5(key ?? '');

  let fromColor = Color(hashStringToColor(hashStr)).saturate(0.5);

  const lightning = ((fromColor as any).hsl() as Color).array()[2];

  if (lightning < 25) {
    fromColor = fromColor.lighten(3);
  }

  if (lightning > 25 && lightning < 40) {
    fromColor = fromColor.lighten(0.8);
  }

  if (lightning > 75) {
    fromColor = fromColor.darken(0.4);
  }

  return {
    from: fromColor.hex(),
    to: getMatchingColor(fromColor).hex(),
  };
}
