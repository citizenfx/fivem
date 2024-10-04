/** @type {import('ts-jest').JestConfigWithTsJest} */
module.exports = {
  "transform": {
    "^.+\\.(t|j)sx?$": "@swc/jest",
  },
  testEnvironment: 'node',
  moduleNameMapper: {
    "^cfx/(.*)$": "<rootDir>/src/cfx/$1",
    "^cfxui-(.*)-loader!(.*)$": "$2",
  },
  modulePathIgnorePatterns: [
    '<rootDir>/build/',
    '<rootDir>/config/',
  ],
};
