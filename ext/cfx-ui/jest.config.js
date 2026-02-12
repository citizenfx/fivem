/** @type {import('ts-jest').JestConfigWithTsJest} */
module.exports = {
  "transform": {
    "^.+\\.(t|j)sx?$": "@swc/jest",
  },
  testEnvironment: 'node',
  moduleNameMapper: {
    "^cfx/(.*)$": "<rootDir>/src/cfx/$1",
    "^cfxui-(.*)-loader!(.*)$": "$2",
    "\\.(css|scss)$": "identity-obj-proxy",
  },
  modulePathIgnorePatterns: [
    '<rootDir>/build/',
    '<rootDir>/config/',
  ],
  transformIgnorePatterns: ['node_modules/(?!(@cfx-dev))'],
};
