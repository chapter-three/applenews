# Apple News

## Installation

Requirements:

-   [Guzzle](https://github.com/guzzle/guzzle) PHP HTTP client

The recommended way to install is using [Composer Manager](https://www.drupal.org/project/composer_manager)

```shell
drush en composer_manager
drush en apple_news
```

## Run Tests

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```
