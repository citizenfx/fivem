module.exports = {
  root: true,
  extends: ['@cfx-dev/eslint-config-ui'],
  rules: {
    '@typescript-eslint/no-invalid-void-type': 'off',
    'default-case': 'off',
    'import/no-cycle': 'off',
    'no-use-before-define': 'off',
    'no-useless-constructor': 'off',
    'no-underscore-dangle': 'off',
    'no-continue': 'off',
    'no-plusplus': 'warn',
    'max-classes-per-file': 'off',
    'no-shadow': 'off',
    'no-param-reassign': ['error', { props: false }],
    'no-restricted-syntax': 'off',
    'class-methods-use-this': 'off',
    'lines-between-class-members': 'off',
    '@stylistic/lines-between-class-members': 'off',
    'import/prefer-default-export': 'off',
    '@typescript-eslint/no-namespace': 'off',
    '@typescript-eslint/no-explicit-any': 'warn',
    '@typescript-eslint/ban-ts-comment': 'off',
    '@typescript-eslint/ban-types': [
      'error',
      {
        types: {
          // un-ban a type that's banned by default
          Function: false,
        },
        extendDefaults: true,
      },
    ],
    'jsx-a11y/no-autofocus': 'off',
  },
};
