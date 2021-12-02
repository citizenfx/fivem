import React from 'react';
import classnames from 'classnames';
import s from './Modal.module.scss';

export namespace SplitHorizontal {
  export interface LayoutProps {
    className?: string,
    children?: [React.ReactComponentElement<typeof Left>, React.ReactComponentElement<typeof Right>],
  }

  export function Layout(props: LayoutProps) {
    const { className, children } = props;

    return (
      <div className={classnames(s['split-horizonal-layout'], className)}>
        {children}
      </div>
    );
  }

  export interface ChildrenProps {
    className?: string,
    children?: React.ReactNode,
  }

  export function Left(props: ChildrenProps) {
    const { className, children } = props;

    return (
      <div className={classnames(s.left, className)}>
        {children}
      </div>
    );
  }

  export function Right(props: ChildrenProps) {
    const { className, children } = props;

    return (
      <div className={classnames(s.right, className)}>
        {children}
      </div>
    );
  }
}
