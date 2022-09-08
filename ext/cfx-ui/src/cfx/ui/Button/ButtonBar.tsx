import s from './Button.module.scss';

export function ButtonBar({ children }) {
  return (
    <div className={s.bar}>
      {children}
    </div>
  );
}
