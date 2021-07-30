import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { TOOL_SIDE, WETool, WEToolbarState } from '../../../store/WEToolbarState';
import s from './BaseTool.module.scss';
import { Title } from 'components/controls/Title/Title';

export interface BaseToolProps {
  tool: WETool,
  icon: React.ReactNode,
  label: string,

  highlight?: boolean,
  renderAlways?: boolean,
  children?: React.ReactNode,

  toggleClassName?: string,
  panelClassName?: string,
}

export const BaseTool = observer(function BaseTool(props: BaseToolProps) {
  const {
    tool,
    icon,
    label,
    children,
    highlight = false,
    renderAlways = false,
    toggleClassName: tClassName = '',
    panelClassName: pClassName = '',

  } = props;

  const toolIsOpen = WEToolbarState.isToolOpen(tool);

  const toggleClassName = classnames(s.toggle, s.hoverable, tClassName, {
    [s.active]: toolIsOpen,
    [s.highlight]: highlight,
  });

  const panelClassName = classnames(s.panel, pClassName, {
    [s.active]: toolIsOpen,
    [s.right]: TOOL_SIDE[tool] === 'right',
  });

  let childrenNode = null;
  if (children && (renderAlways || toolIsOpen)) {
    childrenNode = (
      <div className={panelClassName}>
        {children}
      </div>
    );
  }

  return (
    <>
      <Title animated={false} title={label} delay={0} fixedOn="top">
        {(ref) => (
          <button
            ref={ref}
            className={toggleClassName}
            onClick={() => WEToolbarState.toggleTool(tool)}
          >
            {icon}
          </button>
        )}
      </Title>

      {childrenNode}
    </>
  );
});
