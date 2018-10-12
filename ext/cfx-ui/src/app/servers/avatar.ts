// based on code from https://github.com/tobiaslins/avatar, which is:
/*
Copyright (c) 2017 Tobias Lins

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

import * as Color from 'color';
import * as md5 from 'js-md5';

export class Avatar {
    private static svg = `<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg width="$SIZE" height="$SIZE" viewBox="120 120 120 120" version="1.1" xmlns="http://www.w3.org/2000/svg">
  <g>
    <defs>
      <linearGradient id="avatar" x1="0" y1="0" x2="1" y2="1">
        <stop offset="0%" stop-color="$FIRST"/>
        <stop offset="100%" stop-color="$SECOND"/>
      </linearGradient>
    </defs>
    <rect fill="url(#avatar)" x="120" y="120" width="120" height="120"/>
  </g>
</svg>
`;

    public static getFor(key: string) {
        const hashStr: string = md5(key);

        let firstColor = Color(Avatar.hashStringToColor(hashStr)).saturate(0.5);

        const lightning = (<Color>(<any>firstColor).hsl()).array()[2];
        if (lightning < 25) {
            firstColor = firstColor.lighten(3);
        }
        if (lightning > 25 && lightning < 40) {
            firstColor = firstColor.lighten(0.8);
        }
        if (lightning > 75) {
            firstColor = firstColor.darken(0.4);
        }

        let avatar = Avatar.svg.replace('$FIRST', firstColor.hex());
        avatar = avatar.replace('$SECOND', Avatar.getMatchingColor(firstColor).hex());

        avatar = avatar.replace(/(\$SIZE)/g, '32');

        return avatar;
    }

    private static djb2(str: string) {
        let hash = 5381;
        for (let i = 0; i < str.length; i++) {
            hash = (hash << 5) + hash + str.charCodeAt(i);
        }
        return hash;
    }

    private static hashStringToColor(str: string) {
        const hash = Avatar.djb2(str);
        const r = (hash & 0xff0000) >> 16;
        const g = (hash & 0x00ff00) >> 8;
        const b = hash & 0x0000ff;
        return (
            '#' +
            ('0' + r.toString(16)).substr(-2) +
            ('0' + g.toString(16)).substr(-2) +
            ('0' + b.toString(16)).substr(-2)
        );
    }

    private static getMatchingColor(firstColor: Color) {
        let color = firstColor;
        if (color.isDark()) {
            color = color.saturate(0.3).rotate(90);
        } else {
            color = color.desaturate(0.3).rotate(90);
        }
        if (Avatar.shouldChangeColor(color)) {
            color = color.rotate(-200).saturate(0.5);
        }
        return color;
    }

    private static shouldChangeColor(color: Color) {
        const rgb = color.rgb().array();
        const val = 765 - (rgb[0] + rgb[1] + rgb[2]);
        if (val < 250 || val > 700) {
            return true;
        }
        return false;
    }
}