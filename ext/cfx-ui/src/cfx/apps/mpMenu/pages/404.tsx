import { Navigate } from "react-router-dom";

export function Handle404() {
  return (
    <Navigate to='/' />
  );
}
