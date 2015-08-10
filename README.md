# Apple News

## Installation

You can install with Drush or manually.

To use [Drush](https://github.com/drush-ops/drush):

```shell
drush en apple_news
```

Otherwise, to manually install:

1.  Download this module and its dependencies in the [usual manner](https://www.drupal.org/documentation/install/modules-themes):
    -   [Composer Manager](https://www.drupal.org/project/composer_manager)
    -   [Entity](https://www.drupal.org/project/entity)
2.  Visit `admin/modules` and enable the Apple News module.
3.  Change into the the "Composer File Directory" as configured in Composer Manager's settings page which is where the consolidated composer.json file was generated.
4.  If necessary, [download and install the Composer tool](https://getcomposer.org/doc/00-intro.md#installation-linux-unix-osx).
5.  Run `php composer.phar install --no-dev` on the command line, replace install with update when updating dependencies.

## Run Tests

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```

