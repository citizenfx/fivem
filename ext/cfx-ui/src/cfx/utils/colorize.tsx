import React from 'react';

const colorPrefix = '^';
const colorIndex = Array(10)
  .fill(0)
  .map((_, i) => i.toString())
  .reduce((acc, i) => {
    acc[i] = true;

    return acc;
  }, {});

type Color = string;
type Chunk = string;

export function colorize(str: string): React.ReactNode {
  if (!str) {
    return null;
  }

  const parts: [Color, Chunk][] = [];

  let pos = 0;
  let current: [Color, Chunk] = ['default', ''];

  while (pos < str.length) {
    const char = str[pos++];
    const nextChar = str[pos];

    if (char === colorPrefix) {
      if (colorIndex[nextChar]) {
        pos++;

        parts.push(current);
        current = ['default', ''];

        current[0] = nextChar;
      }
    } else {
      current[1] += char;
    }
  }

  parts.push(current);

  return parts
    .filter((part) => part[1])
    .map(([color, part], index) => {
      if (color === 'default') {
        return part;
      }

      return (
        // eslint-disable-next-line react/no-array-index-key
        <span key={part + color + index} className={`color-${color}`}>
          {part}
        </span>
      );
    });
}
