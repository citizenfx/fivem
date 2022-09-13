import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import { observer } from 'mobx-react-lite';
import s from './Title.module.scss';
import mergeRefs from 'cfx/utils/mergeRefs';
import { TitleOutlet } from '../outlets';

const TITLE_OFFSET = 10;

type FixedOn =
  | 'top' | 'top-left'
  | 'right'
  | 'bottom' | 'bottom-left' | 'bottom-right'
  | 'left';

function getCssStyle(fixed: boolean, delay: number, [x, y]: number[]): React.CSSProperties {
  const top = fixed
    ? y
    : y - 20;
  const left = fixed
    ? x
    : x + 16;

  return {
    // Prefer top and left for element to snap to the pixel grid
    // as when using transform it will become blurry in most of the cases as top and left values are floats
    top: `${top}px`,
    left: `${left}px`,
    // transform: `translate(${left}px, ${top}px)`,
    maxWidth: `calc(100vw - ${left}px - 10px)`,
    animationDelay: `${delay}ms`,
  };
}

function getCoords(element: HTMLElement, fixedOn: FixedOn): [number, number] {
  const { x, y, width, height } = element.getBoundingClientRect();

  let xOffset: number;
  let yOffset: number;

  switch (fixedOn) {
    case 'top': {
      xOffset = x + (width / 2);
      yOffset = y - TITLE_OFFSET;
      break;
    }
    case 'top-left': {
      xOffset = x;
      yOffset = y - TITLE_OFFSET;
      break;
    }

    case 'bottom': {
      xOffset = x + (width / 2);
      yOffset = y + height + TITLE_OFFSET;
      break;
    }
    case 'bottom-left': {
      xOffset = x;
      yOffset = y + height + TITLE_OFFSET;
      break;
    }
    case 'bottom-right': {
      xOffset = x + width;
      yOffset = y + height + TITLE_OFFSET;
      break;
    }

    case 'left': {
      xOffset = x - TITLE_OFFSET;
      yOffset = y + (height / 2);
      break;
    }

    case 'right': {
      xOffset = x + width + TITLE_OFFSET;
      yOffset = y + (height / 2);
      break;
    }
  }

  return [
    xOffset,
    yOffset,
  ];
}

type TitleChildren =
  | ((ref: React.RefObject<any>) => React.ReactNode)
  | React.ReactElement;

export interface TitleProps {
  title?: React.ReactNode,

  delay?: number,
  animated?: boolean,
  fixedOn?: FixedOn,

  children: TitleChildren,
}

export const Title = observer(function Title(props: TitleProps) {
  const {
    title,
    children,
    animated = false,
    fixedOn = 'bottom',
    delay = 0,
  } = props;

  const fixedOnRef = React.useRef(fixedOn);
  fixedOnRef.current = fixedOn;

  const ref = React.useRef<HTMLElement | null>(null);

  const [active, setActive] = React.useState(false);
  const [coords, setCoords] = React.useState([0, 0]);

  React.useEffect(() => {
    if (!ref.current) {
      return;
    }

    const handleMouseEnter = (event: MouseEvent) => {
      if (fixedOnRef.current && ref.current) {
        setCoords(getCoords(ref.current, fixedOnRef.current));
      } else {
        setCoords([event.clientX, event.clientY]);
      }

      setActive(true);
    };

    const handleMouseLeave = () => {
      setActive(false);
    };

    const handleMouseMove = (event: MouseEvent) => {
      if (fixedOnRef.current) {
        return;
      }

      setCoords([event.clientX, event.clientY]);
    };

    ref.current.addEventListener('mouseenter', handleMouseEnter);
    ref.current.addEventListener('mouseleave', handleMouseLeave);
    ref.current.addEventListener('mousemove', handleMouseMove);

    return () => {
      if (!ref.current) {
        return;
      }

      ref.current.removeEventListener('mouseenter', handleMouseEnter);
      ref.current.removeEventListener('mouseleave', handleMouseLeave);
      ref.current.removeEventListener('mousemove', handleMouseMove);
    };
  }, []);

  let titleNode: React.ReactNode = null;
  if (active && title) {
    const wrapperClassName = clsx(s.wrapper, s[`fixed-on-${fixedOn}`], {
      [s.animated]: animated,
    });

    titleNode = (
      <TitleOutlet>
        <div
          className={wrapperClassName}
          style={getCssStyle(!!fixedOn, delay, coords)}
        >
          <div className={s.root}>
            {title}
          </div>
        </div>
      </TitleOutlet>
    );
  }

  const actualChildren = typeof children === 'function'
    ? children(ref)
    : React.cloneElement(children, {
      ref: mergeRefs(ref, children.props.ref),
    });

  return (
    <>
      {actualChildren}
      {titleNode}
    </>
  );
});
