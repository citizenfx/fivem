import { createPlaceholderIconDataURI } from 'cfx/base/placeholderIcon';

import { IServerView } from './types';

export function getServerIconURL(server: IServerView): string {
  if (server.joinId && typeof server.iconVersion === 'number') {
    return `https://servers-frontend.fivem.net/api/servers/icon/${server.joinId}/${server.iconVersion}.png`;
  }

  if (server.historicalIconURL) {
    return server.historicalIconURL;
  }

  return getServerIconPlaceholder(server.id);
}

export function getServerIconPlaceholder(address: string): string {
  return createPlaceholderIconDataURI(address);
}

export async function createServerHistoricalIconURL(server: IServerView): Promise<string> {
  if (server.historicalIconURL) {
    return server.historicalIconURL;
  }

  return createThumbnail(getServerIconURL(server));
}

async function createThumbnail(uri: string, width = 16, height = 16): Promise<string> {
  try {
    const res = await window.fetch(uri);

    if (!res.ok) {
      return '';
    }

    const bitmap: ImageBitmap = await createImageBitmap(await res.blob(), {
      resizeQuality: 'medium',
      resizeWidth: width,
      resizeHeight: height,
    });

    const canvas = document.createElement('canvas');
    canvas.width = bitmap.width;
    canvas.height = bitmap.height;

    const context = canvas.getContext('2d');

    if (!context) {
      return '';
    }

    context.drawImage(bitmap, 0, 0);

    const outBlob = await new Promise<Blob | null>((resolve) => {
      canvas.toBlob(resolve, 'image/png');
    });

    if (!outBlob) {
      return '';
    }

    return new Promise((resolve) => {
      const fileReader = new FileReader();

      fileReader.addEventListener('load', () => {
        resolve(fileReader.result as string);
      });

      fileReader.readAsDataURL(outBlob);
    });
  } catch (e) {
    return '';
  }
}
