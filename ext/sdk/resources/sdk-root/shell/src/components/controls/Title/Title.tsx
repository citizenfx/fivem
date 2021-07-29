import React from 'react';
import ReactDOM from 'react-dom';
import classnames from 'classnames';
import { div } from 'utils/styled';
import s from './Title.module.scss';
import { observer } from 'mobx-react-lite';

const Root = div(s, 'root');
const Shortcut = div(s, 'shortcut');

type FixedOn =
  | 'top'
  | 'right'
  | 'bottom' | 'bottom-left'
  | 'left';

function getCssStyle(fixed: boolean, delay: number, [x, y]: number[]): React.CSSProperties {
  const top = fixed
    ? y
    : y - 20;
  const left = fixed
    ? x
    : x + 16;

  return {
    top: `${top}px`,
    left: `${left}px`,
    maxWidth: `calc(100vw - ${left}px - 10px)`,
    animationDelay: `${delay}ms`,
  };
}

function getCoords(ref: React.RefObject<HTMLElement>, fixedOn: FixedOn): [number, number] {
  const { x, y, width, height } = ref.current.getBoundingClientRect();

  let xOffset: number;
  let yOffset: number;

  switch (fixedOn) {
    case 'top': {
      xOffset = x + (width/2);
      yOffset = y - 10;
      break;
    }

    case 'bottom': {
      xOffset = x + (width/2);
      yOffset = y + height + 10;
      break;
    }
    case 'bottom-left': {
      xOffset = x;
      yOffset = y + height + 10;
      break;
    }

    case 'left': {
      xOffset = x - 10;
      yOffset = y + (height/2);
      break;
    }

    case 'right': {
      xOffset = x + width + 10;
      yOffset = y + (height/2);
      break;
    }
  }

  return [
    xOffset,
    yOffset,
  ];
}

export interface TitleProps {
  title: React.ReactNode,
  shortcut?: string,

  delay?: number,
  animated?: boolean,
  fixedOn?: FixedOn,

  children(ref: React.RefObject<any>): React.ReactNode,
}

export const Title = observer(function Title(props: TitleProps) {
  const {
    title,
    children,
    shortcut,
    animated = true,
    fixedOn,
    delay = 250,
  } = props;

  const fixedOnRef = React.useRef(fixedOn);
  fixedOnRef.current = fixedOn;

  const ref = React.useRef<HTMLElement | null>(null);

  const [active, setActive] = React.useState(false);
  const [outlet, setOutlet] = React.useState<HTMLElement | null>(null);
  const [coords, setCoords] = React.useState([0, 0]);

  const activeRef = React.useRef(active);
  activeRef.current = active;

  const mouseOverRef = React.useRef(null);
  const mouseOutRef = React.useRef(null);

  React.useEffect(() => {
    const handleMouseMove = (event: MouseEvent) => {
      if (activeRef.current && !fixedOnRef.current) {
        setCoords([event.clientX, event.clientY]);
      }
    };
    const handleMouseOver = (event: MouseEvent) => {
      if (!ref.current) {
        return;
      }

      const ourTarget = ref.current === event.target || ref.current.contains(event.target as any);

      if (ourTarget) {
        if (mouseOutRef.current !== null) {
          clearTimeout(mouseOutRef.current);
          mouseOutRef.current = null;
        }

        if (mouseOverRef.current !== null) {
          return;
        }

        mouseOverRef.current = setTimeout(() => {
          if (fixedOnRef.current) {
            setCoords(getCoords(ref, fixedOnRef.current));
          }

          setActive(true);
          activeRef.current = true;

          mouseOverRef.current = null;
        }, 0);
      }
    };
    const handleMouseOut = (event: MouseEvent) => {
      if (!ref.current) {
        return;
      }

      const ourTarget = ref.current === event.target || ref.current.contains(event.target as any);

      if (ourTarget) {
        if (mouseOverRef.current !== null) {
          clearTimeout(mouseOverRef.current);
          mouseOverRef.current = null;
        }

        if (mouseOutRef.current !== null) {
          return;
        }

        mouseOutRef.current = setTimeout(() => {
          setActive(false);
          activeRef.current = false;

          mouseOutRef.current = null;
        }, 0);
      }
    };

    document.body.addEventListener('mousemove', handleMouseMove);
    document.body.addEventListener('mouseover', handleMouseOver);
    document.body.addEventListener('mouseout', handleMouseOut);

    return () => {
      if (mouseOverRef.current !== null) {
        clearTimeout(mouseOverRef.current);
      }
      if (mouseOutRef.current !== null) {
        clearTimeout(mouseOutRef.current);
      }

      document.body.removeEventListener('mousemove', handleMouseMove);
      document.body.removeEventListener('mouseover', handleMouseOver);
      document.body.removeEventListener('mouseout', handleMouseOut);
    };
  }, []);

  React.useLayoutEffect(() => {
    setOutlet(document.getElementById('title-outlet'));
  }, []);

  let titleNode = null;
  if (active && outlet) {
    const wrapperClassName = classnames(s.wrapper, s[`fixed-on-${fixedOn}`], {
      [s.animated]: animated,
    });

    titleNode = ReactDOM.createPortal(
      <div
        className={wrapperClassName}
        style={getCssStyle(!!fixedOn, delay, coords)}
      >
        <Root>
          {title}
        </Root>

        {!!shortcut && (
          <Shortcut>
            {shortcut}
          </Shortcut>
        )}
      </div>,
      outlet,
    );
  }

  return (
    <>
      {children(ref)}
      {titleNode}
    </>
  );
});
