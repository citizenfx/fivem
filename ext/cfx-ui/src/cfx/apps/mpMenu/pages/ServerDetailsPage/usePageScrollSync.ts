import React from 'react';

import { clamp01 } from 'cfx/utils/math';

export function usePageScrollSync(trackingElementRef: React.MutableRefObject<HTMLElement | null>) {
  const [scrollTop, setScrollTop] = React.useState(0);
  const [offsetTop, setOffsetTop] = React.useState(0);

  const handleScroll = React.useCallback((event: WheelEvent) => {
    if (event.target instanceof Element) {
      setScrollTop(event.target.scrollTop);

      const trackingElement = trackingElementRef.current;

      if (trackingElement) {
        const newOffsetTop = (trackingElement.offsetTop - event.target.scrollTop) / trackingElement.offsetHeight;

        setOffsetTop(newOffsetTop);
      }
    }
  }, []);

  const backdropShiftRef = React.useRef(0);

  const navBarShift = clamp01(1 + offsetTop);

  if (navBarShift > 0) {
    backdropShiftRef.current = scrollTop;
  }

  // 102% as a max value to eliminate rounding bugs on some screen resolutions, as in, if 100% then it might end up leaking 1px
  const navBarTranslateY = `${navBarShift * 102}%`;
  const backdropTranslateY = `-${backdropShiftRef.current}px`;

  return {
    navBarTranslateY,
    backdropTranslateY,
    handleScroll,
  };
}
