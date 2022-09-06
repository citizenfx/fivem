import React from "react";
import { clsx } from "cfx/utils/clsx";
import { colorize } from "cfx/utils/colorize";
import s from './ServerTitle.module.scss';

export interface ServerTitleProps {
  title: string,
  size?: 'large' | 'xlarge' | 'xxlarge' | 'xxxlarge',
}

const cached: Record<string, React.ReactNode> = {};

export function ServerTitle(props: ServerTitleProps) {
  const {
    title,
    size = 'large',
  } = props;

  if (!cached[title]) {
    cached[title] = colorize(title);
  }

  const rootClassName = clsx(s.root, s[`size-${size}`]);

  return (
    <span className={rootClassName}>
      {cached[title]}
    </span>
  );
}
