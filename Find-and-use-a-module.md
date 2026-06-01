# Use distributed infrastructure via locally run modules

You've started a local app that runs on Logos. Maybe vibe-coded or [speedrun](Zero-to-Logos-App.md).

Next step is to add capabilities to interact with computers beyond your own.

## Background: Unfortunate Convention

App devs generally add functionality by integrating messaging apis, cloud storage, and other web services, generally enabled by few large providers. This requires everyone to trust these providers to:

- not view your private data
- not share data you'd like to keep private
- not retain your data when you want it to be deleted
- not degrade their service
- not hold you captive and price-gauge
- ...

Doing things the Logos way, you still get app capabilities like messaging, storage, ..., in a way that enables users to co-own the infrastructure and retain better control of your data.

## The future - Logos Core modules!

Get a taste of whats possible with existing modules: see this article https://blog.logos.co/article/logos-basecamp, or read descriptions via [here](README.md#Finding-modules-and-corresponding-repo-urls)

**Modules all the way down...** You are early, modules build on top of modules, organically growing into a dominant ecosystem like Linux.

Using a module from another

Lets add one...

### Delivery

**Delivery Module - High-level message-delivery API**

Logos [logos-delivery-module](https://github.com/logos-co/logos-delivery-module) wraps this [logos-delivery](https://github.com/logos-messaging/logos-delivery) library.

Instructions to use this module can be found [here](https://github.com/logos-co/logos-docs/blob/c85b6bb6fec078f488fb2471a6851652f011ca9c/docs/messaging/journeys/use-the-logos-delivery-module-api-from-an-app.md) (in a [PR](https://github.com/logos-co/logos-docs/pull/226) ready for review).
