# [DRAFT] Apple News

## Before you start

These instructions assume that you have a working Drupal site (brand new or pre-existing) ready to work with, and that you have access to administer modules and add code to the site's code base. To install Drupal locally or on remote server, please see these guides on [Quick installation](https://www.drupal.org/documentation/install) and [Quick installation for developers](https://www.drupal.org/documentation/install/developers).

[Drush](https://github.com/drush-ops/drush) is not mandatory but will make for an easier, and faster installation.

---
## Installation

You can install the apple_news module using Drush or manually adding the modules and libraries to your code base.

#### Drush based installation

To properly run drush commands on your site you must shell into your Drupal sites directory, into the folder that contains the settings.php file specifically for your site. Some examples of what this would be are: 
```shell
sites/default/[location of settings.php file]
sites/mysite.local/[location of settings.php file]
sites/mysuperawesomenewssite.com/[location of settings.php file]
```

**NOTE: Before the module is added to drupal.org, the apple_news module can't be downloaded using [Drush](https://github.com/drush-ops/drush). It must be manually added to the sites/all/modules/contrib directory.**

After adding this module to your code base in the [usual manner](https://www.drupal.org/documentation/install/modules-themes), you can enable it using the drush command:

```shell
drush en apple_news -y
```
This will also automatically download and install the necessary module dependencies for this module. They include the [Libraries](https://www.drupal.org/project/libraries) module and the [Entity API](https://www.drupal.org/project/entity) module.

You will also receive messages to install the necessary AppleNews and php-curl-class libraries. 

To download these necessary libraries, 

1. In your terminal, navigate to your `sites/all/libraries` folder and run the following curl commands to download the proper versions. **NOTE: The AppleNews library is currently gated in a private repo. the curl command won't work until publicly released. To get around this, if you have access to the repo, you can downlaod from [the github page](https://github.com/chapter-three/AppleNews/tree/0.1.9)**
2. Inside of the libraries folder, run the following curl command: 
    
    ```shell
    $ curl -L https://github.com/php-curl-class/php-curl-class/archive/3.5.5.tar.gz | tar xz
    ```
3. Then, run the following curl command:

    ```shell
    $ curl -L https://github.com/chapter-three/AppleNews/archive/0.1.9.zip | tar xz
    ```

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot).

#### Manually based installation

To manually install:

1.  Download this module and its dependencies in the [usual manner](https://www.drupal.org/documentation/install/modules-themes):
    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)
2.  [Download and install the Apple News library](https://github.com/chapter-three/AppleNews/archive/0.1.9.zip) into your libraries folder. **Note: The AppleNews library is currently gated in a private repo. This download may not be accessible unless you have access to the private repository**
3.  [Download and install the PHP Curl Class library (version 3.5.5)](https://github.com/php-curl-class/php-curl-class/archive/3.5.5.tar.gz) into your libraries folder. **Note: Must be version 3.5.5, not higher or lower.**
4.  Visit `admin/modules` and enable the Apple News module. This will enable the Libraries module and the Entities module, as well as any dependencies 

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot)

---
## <a name="configuration"></a>Configuration
Congrats on installing the Apple News module. Please follow these configuration instructions to start publishing your content.

1. task one
2. task two

You are now ready to start sharing your posts and articles with the Apple news app and with the world.

---
## <a name="troubleshoot"></a>Troubleshoot

If you are having trouble installing the module or it's dependcies, review the common scenarios below.

1. Common trouble one
2. Common trouble two

---
## Run Tests

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```

