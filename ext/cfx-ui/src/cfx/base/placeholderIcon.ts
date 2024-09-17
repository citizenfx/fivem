import { getGradientFor } from 'cfx/utils/color';

export function createPlaceholderIconDataURI(id: string): string {
  const icon = createPlaceholderIconSVGString(id);

  if (URL && 'createObjectURL' in URL) {
    return URL.createObjectURL(
      new Blob([icon], {
        type: 'image/svg+xml',
      }),
    );
  }

  return `data:image/svg+xml;base64,${btoa(decodeURIComponent(encodeURIComponent(icon)))}`;
}

export function createPlaceholderIconSVGString(id: string): string {
  const gradient = getGradientFor(id);

  return `<?xml version="1.0" standalone="no"?>
  <!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
  <svg width="32" height="32" viewBox="120 120 120 120" version="1.1" xmlns="http://www.w3.org/2000/svg">
    <g>
      <defs>
        <linearGradient id="avatar" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%" stop-color="${gradient.from}"/>
          <stop offset="100%" stop-color="${gradient.to}"/>
        </linearGradient>
      </defs>
      <rect fill="url(#avatar)" x="120" y="120" width="120" height="120"/>
    </g>
  </svg>
  `;
}
