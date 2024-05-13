import React from 'react';

import { Interactive } from 'cfx/ui/Interactive/Interactive';
import { clsx } from 'cfx/utils/clsx';
import { useInstance } from 'cfx/utils/hooks';
import { clamp01 } from 'cfx/utils/math';

import s from './Scrollable.module.scss';

interface RailProps {
  axis: 'x' | 'y';
  pos: number;
  rootRef: React.RefObject<HTMLDivElement>;
  size: number;
  scrollSize: number;
  minThumbSize: number;
  setRootActive(active: boolean): void;
}

function initRailState() {
  return {
    pos: 0,
    ratio: 1,
    height: 0,
    offset: 0,
    dragging: false,
    dragStartAtCursor: 0,
    dragStartAtPos: 0,

    movementMultiplier: 1,
  };
}

export function Rail(props: RailProps) {
  const {
    axis,
    pos,
    size,
    scrollSize,
    rootRef,
    setRootActive,
    minThumbSize,
  } = props;

  const mutatableProps = React.useMemo(() => {
    if (axis === 'x') {
      return {
        sizeCSS: 'width',
        offsetCSSTransform: 'translateX',
        scrollToSide: 'left',
        mousePosAxis: 'clientX',
      };
    }

    return {
      sizeCSS: 'height',
      offsetCSSTransform: 'translateY',
      scrollToSide: 'top',
      mousePosAxis: 'clientY',
    };
  }, []);

  const [active, setActive] = React.useState(false);

  const state = useInstance(initRailState);

  state.pos = pos;
  state.ratio = clamp01(size / scrollSize);
  state.height = size * state.ratio;
  state.offset = pos * state.ratio;

  if (state.height < minThumbSize) {
    state.height = minThumbSize;
    state.offset = pos * state.ratio * ((size - minThumbSize) / size);
  }

  state.movementMultiplier = 1 / state.ratio;

  const handleMouseDown = React.useCallback((event: React.MouseEvent<HTMLDivElement>) => {
    state.dragging = true;

    state.dragStartAtPos = state.pos;
    state.dragStartAtCursor = event[mutatableProps.mousePosAxis];

    setActive(true);
    setRootActive(true);
  }, []);

  React.useEffect(() => {
    const moveHandler = (event: MouseEvent) => {
      if (!state.dragging) {
        return;
      }

      const movement = event[mutatableProps.mousePosAxis] - state.dragStartAtCursor;

      rootRef.current?.scrollTo({
        [mutatableProps.scrollToSide]: state.dragStartAtPos + movement * state.movementMultiplier,
        behavior: 'auto',
      });
    };

    const upHandler = () => {
      if (!state.dragging) {
        return;
      }

      state.dragging = false;

      setActive(false);
      setRootActive(false);
    };

    window.addEventListener('mousemove', moveHandler);
    window.addEventListener('mouseup', upHandler);

    return () => {
      window.removeEventListener('mousemove', moveHandler);
      window.removeEventListener('mouseup', upHandler);
    };
  }, []);

  const thumbStyle: React.CSSProperties = {
    [mutatableProps.sizeCSS]: `${state.height}px`,
    transform: `${mutatableProps.offsetCSSTransform}(${state.offset}px)`,
  };

  return (
    <div className={s[`rail-${axis}`]}>
      <Interactive
        showPointer={false}
        style={thumbStyle}
        className={clsx(s.thumb, { [s.active]: active })}
        onMouseDown={handleMouseDown}
      />
    </div>
  );
}
