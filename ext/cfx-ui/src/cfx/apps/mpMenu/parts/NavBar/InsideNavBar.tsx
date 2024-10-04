import { observer } from 'mobx-react-lite';
import ReactDOM from 'react-dom';

import { NavBarState } from './NavBarState';

export const InsideNavBar = observer(function InsideNavBar({
  children,
}: ChildrenProps) {
  if (!NavBarState.ready) {
    return null;
  }

  if (!NavBarState.outletRef.current) {
    return null;
  }

  return ReactDOM.createPortal(children, NavBarState.outletRef.current);
});
