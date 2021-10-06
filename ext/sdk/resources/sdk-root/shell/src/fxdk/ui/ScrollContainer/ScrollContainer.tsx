import * as React from 'react';
import classnames from 'classnames';
import 'overlayscrollbars/css/OverlayScrollbars.css';
import './ScrollContainer.scss';
import { OverlayScrollbarsComponent, OverlayScrollbarsComponentProps } from 'overlayscrollbars-react';
import s from './ScrollContainer.module.scss';

export interface ScrollContainerProps {
  className?: string,
  children: React.ReactNode,
  scrollToEnd?: boolean,
}

export const ScrollContainer = React.memo(function ScrollContainer(props: ScrollContainerProps) {
  const { className, children, scrollToEnd = false } = props;

  const ref = React.useRef<OverlayScrollbarsComponent | null>(null);
  const bottomMost = React.useRef<HTMLDivElement | null>(null);

  const rootClassName = classnames(s.root, 'os-theme-fxdk', className);

  React.useLayoutEffect(() => {
    if (bottomMost.current && scrollToEnd) {
      bottomMost.current.scrollIntoView(false);
    }
  }, []);

  const options: OverlayScrollbarsComponentProps["options"] = React.useMemo(() => ({
    scrollbars: {
      autoHide: 'leave',
      autoHideDelay: 0,
    },
  }), []);

  return (
    <OverlayScrollbarsComponent
      ref={ref}
      className={rootClassName}
      options={options}
    >
      {children}
      <div ref={bottomMost} />
    </OverlayScrollbarsComponent>
  );
});
