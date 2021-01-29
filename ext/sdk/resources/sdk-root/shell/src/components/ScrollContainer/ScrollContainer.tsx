import * as React from 'react';
import classnames from 'classnames';
import PerfectScrollbar, { ScrollBarProps } from 'react-perfect-scrollbar';
import 'react-perfect-scrollbar/dist/css/styles.css';
import s from './ScrollContainer.module.scss';

export const ScrollContainer = React.memo(function ScrollContainer(props: ScrollBarProps) {
  const { className, children, ...restProps } = props;

  const rootClassName = classnames(s.root, className);

  return (
    <PerfectScrollbar
      className={rootClassName}
      {...restProps}
    >
      {children}
    </PerfectScrollbar>
  );
})
