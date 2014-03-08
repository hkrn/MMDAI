---
layout: page
title: マニュアル
header: マニュアル
group: navigation
---
{% include JB/setup %}

VPVM {{ site.data.VPVM.version }} のページ
---------------------------------------------

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
