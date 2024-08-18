/* eslint-disable react/no-unescaped-entities */
import s from './DesignShowcase.module.scss';

export function DesignShowcase() {
  return (
    <div className={s.root}>
      {/* Offsets */}
      <section>
        <h1>Offsets</h1>

        <div>
          <h4>
            <abbr title="small">Small</abbr> - <kbd>ui.offset('small')</kbd>
          </h4>
          <aside>
            <div className={s.offset} style={{ height: 'var(--offset-small)' }} />
          </aside>
        </div>
        <div>
          <h4>
            <abbr title="normal">Normal</abbr>
          </h4>
          <aside>
            <div className={s.offset} style={{ height: 'var(--offset-normal)' }} />
          </aside>
        </div>
        <div>
          <h4>
            <abbr title="large">Large</abbr>
          </h4>
          <aside>
            <div className={s.offset} style={{ height: 'var(--offset-large)' }} />
          </aside>
        </div>
      </section>

      {/* Font sizes */}
      <section>
        <h1>Font sizes</h1>

        <div>
          <h4>
            Extra small text <kbd>@include ui.font-size('xsmall')</kbd>
          </h4>
          <aside>
            <span style={{ fontSize: 'var(--font-size-xsmall)' }}>Extra small text</span>
          </aside>
        </div>
        <div>
          <h4>
            Small text <kbd>@include ui.font-size('small')</kbd>
          </h4>
          <aside>
            <span style={{ fontSize: 'var(--font-size-small)' }}>Small text</span>
          </aside>
        </div>
        <div>
          <h4>
            Normal text <kbd>@include ui.font-size('normal')</kbd>
          </h4>
          <aside>
            <span style={{ fontSize: 'var(--font-size-normal)' }}>Normal text</span>
          </aside>
        </div>
        <div>
          <h4>
            Large text <kbd>@include ui.font-size('large')</kbd> or <kbd>font-size: var(--font-size-large)</kbd>
          </h4>
          <aside>
            <span style={{ fontSize: 'var(--font-size-large)' }}>Large text</span>
          </aside>
        </div>

        <ul>
          <li>
            <span style={{ fontSize: 'var(--font-size-xsmall)' }}>Xsmall text</span>
          </li>
          <li>
            <span style={{ fontSize: 'var(--font-size-small)' }}>Small text</span>
          </li>
          <li>
            <span style={{ fontSize: 'var(--font-size-normal)' }}>Normal text</span>
          </li>
          <li>
            <span style={{ fontSize: 'var(--font-size-large)' }}>Large text</span>
          </li>
        </ul>
      </section>

      {/* Font families */}
      <section>
        <h1>Font families</h1>

        <div>
          <h4>
            Primary, Rubik <kbd>font-family: var(--font-family-primary)</kbd>
          </h4>
          <p>
            Everyhing <em>textual</em> except for headers.
          </p>
          <aside>
            <span style={{ fontFamily: 'var(--font-family-primary)' }}>Primary font text</span>
          </aside>
        </div>
        <div>
          <h4>
            Secondary, Montserrat <kbd>font-family: var(--font-family-secondary)</kbd>
          </h4>
          <p>For headers</p>
          <aside>
            <h1>H1 Header</h1>
            <h2>H2 Header</h2>
            <h3>H3 Header</h3>
            <h4>H4 Header</h4>
            <h5>H5 Header</h5>
            <h6>H6 Header</h6>
          </aside>
        </div>
      </section>

      {/* Colors */}
      <section>
        <h1>Colors</h1>

        <p>
          Get color using <kbd>ui.color(name, [luminance, [alpha]])</kbd>
          <br />
          Pure variant of color is default luminance. To get background with 50% opacity:{' '}
          <kbd>ui.color('bg', 'pure', .5)</kbd>
        </p>

        <div>
          <h4>
            Foreground - <kbd>fg</kbd>
          </h4>
          <ColorsRow color="fg" />
        </div>
        <div>
          <h4>
            Background - <kbd>bg</kbd>
          </h4>
          <ColorsRow color="bg" />
        </div>
        <div>
          <h4>
            Primary - <kbd>primary</kbd>
          </h4>
          <ColorsRow color="primary" />
        </div>
        <div>
          <h4>
            Success - <kbd>success</kbd>
          </h4>
          <ColorsRow color="success" />
        </div>
        <div>
          <h4>
            Warning - <kbd>warning</kbd>
          </h4>
          <ColorsRow color="warning" />
        </div>
        <div>
          <h4>
            Error - <kbd>error</kbd>
          </h4>
          <ColorsRow color="error" />
        </div>
      </section>
    </div>
  );
}

function ColorsRow({
  color,
}: { color: 'fg' | 'bg' | 'primary' | 'secondary' | 'success' | 'warning' | 'error' }) {
  return (
    <div className={s.row}>
      <div className={s.square} style={{ backgroundColor: `rgb(var(--color-${color}))` }}>
        pure
      </div>

      <div />

      <div className={s.square} style={{ backgroundColor: `rgb(var(--color-${color}-50))` }}>
        50
      </div>
      {Array(9)
        .fill(0)
        .map((_, i) => (
          // eslint-disable-next-line react/no-array-index-key
          <div key={i} className={s.square} style={{ backgroundColor: `rgb(var(--color-${color}-${(i + 1) * 100}))` }}>
            {(i + 1) * 100}
          </div>
        ))}

      <div />

      <div className={s.square} style={{ backgroundColor: `rgb(var(--color-${color}-950))` }}>
        950
      </div>
    </div>
  );
}
