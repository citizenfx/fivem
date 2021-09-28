import React from 'react';
import classnames from 'classnames';
import { noop } from 'utils/noop';
import s from './PropertiesTool.module.scss';
import { BsChevronDown, BsChevronRight } from 'react-icons/bs';

export interface PropertiesToolSectionProps {
  open: boolean,
  title: React.ReactNode,
  children: React.ReactNode,

  onOpenToggle?: () => void,
}

export const PropertiesToolSection = function PropertiesToolSection(props: PropertiesToolSectionProps) {
  const { open, title, children, onOpenToggle = noop } = props;

  const sectionClassName = classnames(s.section, {
    [s.open]: open,
  });

  return (
    <div className={sectionClassName}>
      <div className={s.toggle} onClick={onOpenToggle}>
        <div className={s.icon}>
          {
            open
              ? <BsChevronDown />
              : <BsChevronRight />
          }
        </div>
        <span>
          {title}
        </span>
      </div>

      <div className={s.content}>
        {children}
      </div>
    </div>
  );
}
