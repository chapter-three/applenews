# Apple News

## Installation

You can install with Drush or manually.

To use [Drush](https://github.com/drush-ops/drush):

```shell
drush en apple_news
```

Otherwise, to manually install:

1.  Download this module and its dependencies in the [usual manner](https://www.drupal.org/documentation/install/modules-themes):
    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)
2.  [Download and install the Apple News library](https://github.com/chapter-three/AppleNews) into your libraries folder.
3.  [Download and install the PHP Curl Class library (version 3.5.5)](https://github.com/php-curl-class/php-curl-class/tree/3.5.5) into your libraries folder. **Must be version 3.5.5 or above**
4.  Visit `admin/modules` and enable the Apple News module.

## Run Tests

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```

