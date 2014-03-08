---
layout: page
title: マニュアル
header: マニュアル
group: navigation
---
{% include JB/setup %}

VPVM {{ site.data.VPVM.version }} のページ
---------------------------------------------

[取扱説明書的なページ](https://github.com/hkrn/MMDAI/wiki/VPVM) は github 上にあります。

<ul>
  {% assign pages_list = site.tags.VPVM %}
  {% include JB/pages_list %}
</ul>

MMDAI2 0.30.0 のページ
--------------------------

<ul>
  {% assign pages_list = site.pages %}
  {% assign group = "manual" %}
  {% include JB/pages_list %}
</ul>
