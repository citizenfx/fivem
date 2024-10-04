import { Navigate, useLocation } from 'react-router-dom';

export function Handle404() {
  const loc = useLocation();

  console.warn('No such page', loc.pathname);

  return (
    <Navigate to="/" />
  );
}
