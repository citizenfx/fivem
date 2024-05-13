import React from 'react';

import { useContextualStyle } from 'cfx/ui/Style/Style';
import { clsx } from 'cfx/utils/clsx';
import mergeRefs from 'cfx/utils/mergeRefs';

import { Rail } from './Rail';

import s from './Scrollable.module.scss';

export interface ScrollableProps {
  minThumbSize?: number;
  className?: string;
  children?: React.ReactNode;
  onScroll?: (event: WheelEvent) => void;
  scrollerRef?: React.Ref<HTMLDivElement>;

  /**
   * If true, will repurpose wheel events for horizontal scroll instead
   */
  verticalAsHorizontal?: boolean;
}

interface IAxisState {
  size: number;
  scrollPos: number;
  scrollSize: number;
}

const INITIAL_AXIS_STATE: IAxisState = {
  size: 0,
  scrollPos: 0,
  scrollSize: 0,
};

export const Scrollable = React.forwardRef(function Scrollable(props: ScrollableProps, ref: React.Ref<HTMLDivElement>) {
  const {
    children,
    className,
    onScroll,
    minThumbSize = 20,
    verticalAsHorizontal = false,
  } = props;

  const scrollerRef = React.useRef<HTMLDivElement>(null);
  const contentRef = React.useRef<HTMLDivElement>(null);

  const [active, setActive] = React.useState(false);

  const [xState, setXState] = React.useState(INITIAL_AXIS_STATE);
  const [xPos, setXPos] = React.useState(0);

  const [yState, setYState] = React.useState(INITIAL_AXIS_STATE);
  const [yPos, setYPos] = React.useState(0);

  const rootClassName = clsx(s.root, className, {
    [s.active]: active,
  });

  // Observing size changes
  React.useLayoutEffect(() => {
    if (!contentRef.current) {
      throw new Error('No scrollable content ref');
    }

    const observer = new ResizeObserver(() => {
      setYState({
        size: scrollerRef.current?.clientHeight || 0,
        scrollPos: scrollerRef.current?.scrollTop || 0,
        scrollSize: scrollerRef.current?.scrollHeight || 0,
      });

      setXState({
        size: scrollerRef.current?.clientWidth || 0,
        scrollPos: scrollerRef.current?.scrollLeft || 0,
        scrollSize: scrollerRef.current?.scrollWidth || 0,
      });
    });

    observer.observe(contentRef.current);

    return () => observer.disconnect();
  }, []);

  // Handling scroll events
  const handleScroll = React.useCallback(
    (event) => {
      if (!scrollerRef.current) {
        return;
      }

      onScroll?.(event);

      setYPos(scrollerRef.current.scrollTop);
      setXPos(scrollerRef.current.scrollLeft);
    },
    [verticalAsHorizontal],
  );

  // Handling repurposed Y scroll
  React.useEffect(() => {
    if (!verticalAsHorizontal) {
      return;
    }

    if (!scrollerRef.current) {
      return;
    }

    const handler = (event: WheelEvent) => {
      event.preventDefault();
      event.stopPropagation();

      if (scrollerRef.current) {
        scrollerRef.current.scrollLeft += event.deltaY;
      }
    };

    scrollerRef.current.addEventListener('wheel', handler);

    return () => scrollerRef.current?.removeEventListener('wheel', handler);
  }, [verticalAsHorizontal]);

  const showXRail = xState.scrollSize - xState.size > 1;
  const showYRail = yState.scrollSize - yState.size > 1;

  const scrollerClassName = clsx(s.scroller, {
    [s['no-x-scroll']]: !showXRail,
    [s['no-y-scroll']]: !showYRail,
  });

  return (
    <div ref={ref} style={useContextualStyle()} className={rootClassName}>
      <div ref={mergeRefs(scrollerRef, props.scrollerRef)} className={scrollerClassName} onScroll={handleScroll}>
        <div ref={contentRef} className={s.content}>
          {children}
        </div>
      </div>

      {showYRail && (
        <Rail
          rootRef={scrollerRef}
          axis="y"
          size={yState.size}
          pos={yPos}
          scrollSize={yState.scrollSize}
          setRootActive={setActive}
          minThumbSize={minThumbSize}
        />
      )}

      {showXRail && (
        <Rail
          rootRef={scrollerRef}
          axis="x"
          size={xState.size}
          pos={xPos}
          scrollSize={xState.scrollSize}
          setRootActive={setActive}
          minThumbSize={minThumbSize}
        />
      )}
    </div>
  );
});
