# Rime
Code for the conference paper "Streaming Data through the IoT via Actor-Based Semantic Routing Trees" from VLIOT@VLDB 2021.

## License
The content of this project itself, including the source code for the paper (`paper/`) and the log files (`benchmarks/logs/`), is licensed under the [Creative Commons Attribution 3.0 Unported license](https://creativecommons.org/licenses/by/3.0/), and the source code residing in `code/` and `benchmarks/notebooks` is licensed under the [MIT license](code/LICENSE.md).

## Cite this work
```
@Article{OJIOT_2021v7i1n06_Giouroukis,
  title     = {Streaming Data through the IoT via Actor-Based Semantic Routing Trees},
  author    = {Dimitrios Giouroukis and
               Johannes Jestram and
               Steffen Zeuch and
               Volker Markl},
  journal   = {Open Journal of Internet Of Things (OJIOT)},
  issn      = {2364-7108},
  year      = {2021},
  volume    = {7},
  number    = {1},
  pages     = {59--70},
  url       = {https://www.ronpub.com/ojiot/OJIOT_2021v7i1n06_Giouroukis.html},
  publisher = {RonPub},
  bibsource = {RonPub},
  abstract = {The Internet of Things (IoT) enables the usage of resources at the edge of the network for various data management tasks that are traditionally executed in the cloud. However, the heterogeneity of devices and communication methods in a multi-tiered IoT environment (cloud/fog/edge) exacerbates the problem of deciding which nodes to use for processing and how to route data. In addition, both decisions cannot be made only statically for the entire lifetime of an application, as an IoT environment is highly dynamic and nodes in the same topology can be both stationary and mobile as well as reliable and volatile. As a result of these different characteristics, an IoT data management system that spans across all tiers of an IoT network cannot meet the same availability assumptions for all its nodes. To address the problem of choosing ad-hoc which nodes to use and include in a processing workload, we propose a networking component that uses a-priori as well as ad-hoc routing information from the network. Our approach, called Rime, relies on keeping track of nodes at the gateway level and exchanging routing information with other nodes in the network. By tracking nodes while the topology evolves in a geo-distributed manner, we enable efficient communication even in the case of frequent node failures. Our evaluation shows that Rime keeps in check communication costs and message transmissions by reducing unnecessary message exchange by up to 82:65\%.}
}
```
