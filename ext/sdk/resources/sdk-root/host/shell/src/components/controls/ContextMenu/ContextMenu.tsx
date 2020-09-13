import React, { MouseEvent } from 'react';
import ReactDOM from 'react-dom';
import classnames from 'classnames';
import s from './ContextMenu.module.scss';


const noop = () => {};

export interface ContextMenuItem {
  id: string,
  text: string,
  icon: React.ReactNode,
  onClick: () => void,
  disabled?: boolean,
}

export interface ContextMenuProps {
  children: React.ReactNode,
  items: ContextMenuItem[],
  disabled?: boolean,
  className?: string,
  activeClassName?: string,
  onClick?: () => void,
}

interface Coords {
  top: number,
  left: number,
}

export const ContextMenu = React.memo((props: ContextMenuProps) => {
  const {
    children,
    items,
    className = '',
    activeClassName = '',
    onClick = noop,
  } = props;

  const [coords, setCoords] = React.useState<Coords | null>(null);
  const [outlet, setOutlet] = React.useState<HTMLElement | null>(null);

  React.useLayoutEffect(() => {
    setOutlet(document.getElementById('context-menu-outlet'));
  }, []);

  const handleOpenMenu = React.useCallback((event: MouseEvent) => {
    event.preventDefault();
    event.stopPropagation();

    setCoords({
      top: event.pageY,
      left: event.pageX,
    });

    return false;
  }, []);
  const handleCloseMenu = React.useCallback(() => {
    setCoords(null);
  }, []);

  const handleClick = React.useCallback(() => {
    if (!coords) {
      onClick();
    }
  }, [onClick, coords]);

  let menu: React.ReactPortal | null = null;
  if (coords && outlet) {
    const itemsNodes = items.map((item) => (
      <div
        key={item.id}
        className={classnames(s.item, { [s.disabled]: item.disabled })}
        onClick={item.onClick}
      >
        {item.icon}
        {item.text}
      </div>
    ));

    menu = ReactDOM.createPortal(
      <div className={s.backdrop} onClick={handleCloseMenu}>
        <div className={s.menu} style={{ top: coords.top + 'px', left: coords.left + 'px' }}>
          {itemsNodes}
        </div>
      </div>,
      outlet,
    );
  }

  return (
    <div
      className={classnames(className, { [activeClassName]: !!coords })}
      onContextMenu={handleOpenMenu}
      onClick={handleClick}
    >
      {children}
      {menu}
    </div>
  );
});
