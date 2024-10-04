import { decode } from 'blurhash';
import React from 'react';

import { Deferred } from './async';

export function decodeBlurhashToURL(blurhash: string, width: number, height: number, punch = 1): Promise<string> {
  const deferred = new Deferred<string>();

  // decode hash
  const pixels = decode(blurhash, width, height, punch);

  // temporary canvas to create a blob from decoded ImageData
  const canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;

  const ctx = canvas.getContext('2d');

  if (!ctx) {
    deferred.reject(new Error('Failed to get 2D context'));

    return deferred.promise;
  }

  const imageData = ctx.createImageData(width, height);
  imageData.data.set(pixels);

  ctx.putImageData(imageData, 0, 0);

  canvas.toBlob((blob) => {
    if (blob) {
      deferred.resolve(URL.createObjectURL(blob));
    } else {
      deferred.reject(new Error('Failed to grab BLOB from canvas'));
    }
  });

  return deferred.promise;
}

export function useBlurhash(blurhash: string, width: number, height: number, punch?: number) {
  const [url, setUrl] = React.useState<string | null>(null);
  const urlRef = React.useRef(url);
  urlRef.current = url;

  React.useEffect(() => {
    let isCancelled = false;

    decodeBlurhashToURL(blurhash, width, height, punch).then((urlDecoded) => {
      if (isCancelled) {
        if (urlRef.current) {
          URL.revokeObjectURL(urlRef.current);
        }

        return;
      }

      setUrl((oldUrl) => {
        if (oldUrl) {
          URL.revokeObjectURL(oldUrl);
        }

        return urlDecoded;
      });
    });

    return () => {
      isCancelled = true;

      if (urlRef.current) {
        URL.revokeObjectURL(urlRef.current);
      }
    };

    // // decode hash
    // const pixels = decode(blurhash, width, height, punch);

    // // temporary canvas to create a blob from decoded ImageData
    // const canvas = document.createElement('canvas');
    // canvas.width = width;
    // canvas.height = height;

    // const ctx = canvas.getContext('2d');
    // if (!ctx) {
    //   return;
    // }

    // const imageData = ctx.createImageData(width, height);
    // imageData.data.set(pixels);

    // ctx.putImageData(imageData, 0, 0);

    // canvas.toBlob((blob) => {
    //   if (!isCancelled && blob) {
    //     setUrl((oldUrl) => {
    //       if (oldUrl) {
    //         URL.revokeObjectURL(oldUrl);
    //       }

    //       return URL.createObjectURL(blob);
    //     });
    //   }
    // });

    // return () => {
    //   isCancelled = true;

    //   setUrl((oldUrl) => {
    //     if (oldUrl) {
    //       URL.revokeObjectURL(oldUrl);
    //     }

    //     return null;
    //   });
    // };
  }, [blurhash, height, width, punch]);

  return url;
}
