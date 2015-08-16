# [DRAFT] Apple News

#### Table of Contents

1. [Before you Start](#before)
2. [Minimum Requirements](#minimum-req)
3. [Installation](#installation)
    1. [Drush based installation](#drush-install)
    2. [Manually based installation](#manual-install)
4. [Configuration](#configuration)
    1. [Initial settings configuration](#initial)
    2. [Channel/Channel settings](#channel)
    3. [Node configuration](#node)
    4. [Preview post](#preview)
    5. [Delete from channel](#delete)
4. [Troubleshoot](#troubleshoot)
5. [Run Tests](#run-tests)

## <a name="before"></a>Before you start

These instructions assume that you have a working Drupal 7 site (brand new or pre-existing) ready to work with, that you have access to administer modules and add code to the site's code base, And that you meet the minimum requirements for installing this module. Please confirm that you have the [Minimum Requirements](#minimum-req) before proceeding with this installation.

To install Drupal locally or on remote server, please see these guides on [Quick installation](https://www.drupal.org/documentation/install) and [Quick installation for developers](https://www.drupal.org/documentation/install/developers).

[Drush](https://github.com/drush-ops/drush) is not mandatory but will make for an easier, and faster installation.

## <a name="minimum-req"></a>Minimum Requirements

Please make sure that your Drupal site matches these minimum requirements, in order to avoid installation problems.

* **PHP version 5.4 and above.** Version 5.3, and below, will cause fatal error.
* **Drupal 7.** Latest version of Drupal always recommended.
* **Curl enabled for php**. One way to check is to install your drupal site and then visit (`admin/reports/status/php`), and check for 'curl'. If not enabled, a quick google search with 'enable curl php [your server type]' will help.
* **(Optional) Drush 5.9 and above**. You will need Drush installed to follow the [Drush based installation](#drush-install) guide, Otherwise, you will have to follow the [Manually based installation](#manual-install) guide

## <a name="installation"></a>Installation

You can install the apple_news module using Drush or manually adding the modules and libraries to your code base.

### <a name="drush-install"></a>Drush based installation

To properly run drush commands on your site you must shell into your Drupal sites directory, into the folder that contains the settings.php file specifically for your site. Some examples of what this would be are:

```shell
sites/default/[location of settings.php file]
sites/mysite.local/[location of settings.php file]
sites/mysuperawesomenewssite.com/[location of settings.php file]
```

To install Apple News using drush and command line:


1. First, download and enable the module dependencies for this module.

    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)

    ```shell
    $ drush dl libraries entity
    $ drush en libraries entity
    ```

2. Next, in your terminal, navigate to your `sites/all/libraries` folder and run the following curl commands to download the proper library versions. If the `libraries` folder does not exist, please create it before running the following command:

    ```shell
    $ curl -L https://github.com/php-curl-class/php-curl-class/archive/4.6.8.tar.gz | tar xz
    $ mv php-curl-class-4.6.8 php-curl-class
    ```

    **NOTE: The AppleNews library is currently gated in a private repo. the curl command won't work until publicly released. To get around this, if you have access to the repo, you can download from [the github page](https://github.com/chapter-three/AppleNews/tree/0.2.4)**

3. Next, still inside the libraries folder, run the following curl command:

    ```shell
    $ curl -L https://github.com/chapter-three/AppleNews/archive/0.2.4.tar.gz | tar xz
    $ mv AppleNews-0.2.4 AppleNews
    ```

4. After the libraries are downloaded, your should see directories matching the following setups (You may need to rename the folders you just downloaded to match):

    ```
    sites/all/libraries/AppleNews/[files start here]
    sites/all/libraries/php-curl-class/[files start here]
    ```

5. Add the apple_news module to your code base in the [usual manner](https://www.drupal.org/documentation/install/modules-themes), enable it using the drush command:

    **NOTE: Before the module is added to drupal.org, the apple_news module can't be downloaded using [Drush](https://github.com/drush-ops/drush). It must be manually added to the sites/all/modules/contrib directory. If you have access to the repo, you can download the module from [the github page](https://github.com/chapter-three/apple_news)**

    ```shell
    $ drush en apple_news -y
    ```

If you enable the module before downloading the required libraries, you will receive error messages telling you download them. Please try re-installing the libraries or check out the [Troubleshooting Section](#troubleshooting).

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot).

### <a name="manual-install"></a>Manually based installation

To manually install:

1.  Download this module and its dependencies in the [usual manner](https://www.drupal.org/documentation/install/modules-themes):
    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)
2.  Next, [Download and install the Apple News library (version 0.2.4)](https://github.com/chapter-three/AppleNews/archive/0.2.4.zip) into your libraries folder. If you do not have a sites/all/libraries folder, please create it before downloading **Note: The AppleNews library is currently gated in a private repo. This download may not be accessible unless you have access to the private repository**
3.  [Download and install the PHP Curl Class library (version 4.6.8)](https://github.com/php-curl-class/php-curl-class/archive/4.6.8.tar.gz) into your libraries folder. **Note: Must be version 4.6.8, not higher or lower.**
4. After the libraries are downloaded, you should see directories matching the following setups (You may need to rename the folders you just downloaded to match):

    ```shell
    sites/all/libraries/AppleNews/[files start here]
    sites/all/libraries/php-curl-class/[files start here]
    ```

5.  Visit `admin/modules` and enable the Apple News module. This will enable the Libraries module and the Entities module, as well as any other additional dependencies.

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot)


## <a name="configuration"></a>Configuration

Congrats on installing the Apple News module. Please follow these configuration instructions to start publishing your content.

### <a name="initial"></a>Initial settings configuration

1. Visit apple.com to get your credentials and create a news channel that your Drupal site will use.
2. In your Drupal site, navigate to the "Apple news credentials page" (`admin/config/content/apple-news/settings`) and add your Apple News credentials.
3. In your Drupal site, navigate to the "Apple news channels page" (`admin/config/content/apple-news/settings/channels`) and add a channel ID from your apple account. Please add one ID at a time. The channels are validated by the apple credentials you added to your Drupal site, and if valid, will fetch the channel information and add them to your sites list of channels.

### <a name="channel"></a>Default Channel/Export configuration

1. In your Drupal site, navigate to the "Apple news export manager page" (`admin/config/content/apple-news`).
2. Click on the **'edit'** link of the channel you would like to connect to an apple news channel. The default export that comes with the Apple News module is "Nodes", but other exports can be created in code, using custom modules. For an example module, check out the "apple_news_article" module that comes with the Apple News module.
3. In this "Edit page", the minimum requirements to properly configure a channel to an apple news channel are:
    1. Under "Add new component", select "Body(apple_news)".
    2. Under "Channels", select the channel (Apple News Channel) that this export will be tied to. (in other words, this export channel will get nodes, process them, and send them to this selected channel for display in the Apple News app.)
    3. Under "Content types", select the content types that should be processed with this channel.
    4. Under "Layout", the default value is "Simple", but it is possible to create new layouts using a custom module.
    5. Click **Save Changes**
    6. After saving, you will see some options to the right of the new components we just added. These are "edit" and "delete". Click on **"edit"**
    7. Select the source field for this component. This is where we tell apple news that this apple news component will be getting it's data from this Drupal field.
    8. Click **Save Changes**

### <a name="node"></a>Node configuration

Once a content type is enabled in an export/channel, the option to add the individual post reside in the nodes add/edit page. If a content type is not added to any channel export, these options will not be available on the node add/edit page.

1. To add a node to the channel sent to apple, In the "Apple News" tab, select _"Publish to Apple"_. If you want to temporarily stop publishing to apple, or make revisions to the post before publishing or re-publishing to apple, deselect this checkbox. It is the equivalent to the "Publish/Unpublish" feature with drupal nodes.
2. Next, select one or more channels from the available list.
2. And for each selected channel, select an available "Section" that it belongs to. ("Sections" are created on apple.com, where you initially created the channel).
3. Once a node is initially published to an apple news channel, It will also display a general information section showing Post date, Share URL, the Section, and the Channel its published to.

### <a name="preview"></a>Preview a post before publishing

If you want to preview a post before sending it to apple, You will need to first download and install the Apple "News Preview" Application (LINK TBD).

1. After saving the node, return to the nodes edit page
2. Find the "Apple News" Tab, and click the "Download" link under "Preview". This will download a folder containing the specialy formatted file needed by the News Preview App.
2. Drag the whole folder into the App icon to open, and it will display the page just as the Apple News App will be displaying it.

### <a name="delete"></a>Delete a post from publishing

If you want to delete a post from a channel, but not delete the post itself, There is a **delete** link in the "Apple News" tab.


##Congrats!

You are now ready to start sharing your posts and articles with the Apple News Service and with the world. Happy Posting!


## <a name="troubleshoot"></a>Troubleshoot

If you are having trouble installing the module or it's dependencies, review the common scenarios below.

**Problem:** I'm getting the error message that includes:

```shell
Fatal error: undefined '['
```

**Solution:** This means that your version of PHP does not meet minimum standards. Version 5.3 and below are not able to process the bracketed php formatting of the AppleNews library. Updating your version of PHP to 5.4 and above will fix this.

---

**Problem:** I'm getting the error message that includes:

```shell
SSL certificate problem: unable to get local issuer certificate
```

**Solution:** This is a mis-configuration in your server setup. Depending on what server OS you are using, the fix is different. Please see this StackOverflow post "[curl: (60) SSL certificate : unable to get local issuer certificate](http://stackoverflow.com/questions/24611640/curl-60-ssl-certificate-unable-to-get-local-issuer-certificate)" For more information on possible fixes specific for your system

---

**Problem:** I'm getting the error message:

```shell
Please download PHP-Curl-Class (version 4.6.8) library to sites/all/libraries/php-curl-class
```

**Solution:** This means that the library has not been downloaded, the wrong version is in place, or the folder for the library is labeled wrong. Double check that the library was downloaded into `sites/all/libraries/php-curl-class/[files start here]`. Check that the version is 4.6.8 by opening up the composer.json file and search for "version": "4.6.8". Lastly, if still not resolved, make sure the folder is named `php-curl-class` and **NOT** something like `php-curl-class-master` or `php-curl-class-4.6.8`.

---

**Problem:** I'm getting the error message:

```shell
Please download AppleNews (version 0.2.4) library to sites/all/libraries/AppleNews
```

**Solution:** This means that the library has not been downloaded, the wrong version is in place, or the folder for the library is labeled wrong. Double check that the library was downloaded into `sites/all/libraries/AppleNews/[files start here]`. Check that the version is 0.2.4 by opening up the composer.json file and search for "version": "0.2.4". Lastly, if still not resolved, make sure the folder is named `AppleNews` and **NOT** something like `AppleNews-master` or `AppleNews-0.2.4`.

---

Please consult drupal.org for any issues outside of this scope.

## <a name="run-tests"></a>Run Tests

To run Drupal testing, Enable the core "Testing" module, from the Modules admin page or with command line.

To enable and run tests from the UI:

1. Navigate to the Testing admin page (`admin/config/development/testing`).
2. Select "Apple News" from the list of tests
3. Click **Run Tests**

To run test from command line, enter the following commands one at a time:

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```

